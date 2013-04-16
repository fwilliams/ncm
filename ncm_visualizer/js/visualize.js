var scale = 0.2;
var percentile = 0.50;
var cached_data;

// copied from parse.py TODO: move all length calculations into the same file?
lengths = {'FUTURE': {'mew': 200, 'sigma': 100},
           'HALT': {'mew': 200, 'sigma': 100},
           'IF': {'mew': 200, 'sigma': 100},
           'MODE': {'mew': 200, 'sigma': 100},
           'CREATE': {'mew': 200, 'sigma': 100},
           'DESTROY': {'mew': 200, 'sigma': 100},
           'SEND': {'mew': 200, 'sigma': 100},
           'RECEIVE': {'mew': 200, 'sigma': 100},
           'SYNC': {'mew': 200, 'sigma': 100},
           'HANDLE': {'mew': 200, 'sigma': 100},
           'NOP': {'mew': 200, 'sigma': 100},
           'SET_COUNTER': {'mew': 200, 'sigma': 100},
           'ADD_TO_COUNTER': {'mew': 200, 'sigma': 100},
           'SUB_FROM_COUNTER': {'mew': 200, 'sigma': 100},
           'PAUSE': {'mew': 'n/a', 'sigma': 'n/a'},
           'LOOP': {'mew': 'n/a', 'sigma': 'n/a'}};

legend = {'FUTURE': {'bg': '#295f99', 'fg': 'white'},
           'HALT': {'bg': '#ff7f00', 'fg': 'black'},
           'IF': {'bg': '#ffff00', 'fg': 'black'},
           'MODE': {'bg': '#7f007f', 'fg': 'black'},
           'CREATE': {'bg': '#00a833', 'fg': 'black'},
           'DESTROY': {'bg': '#ff0000', 'fg': 'black'},
           'SEND': {'bg': '#94afcc', 'fg': 'black'},
           'RECEIVE': {'bg': '#994b00', 'fg': 'white'},
           'SYNC': {'bg': '#590b3f', 'fg': 'white'},
           'HANDLE': {'bg': '#ffff7f', 'fg': 'black'},
           'NOP': {'bg': '#148366', 'fg': 'black'},
           'SET_COUNTER': {'bg': '#ff3f00', 'fg': 'black'},
           'ADD_TO_COUNTER': {'bg': '#7fd319', 'fg': 'black'},
           'SUB_FROM_COUNTER': {'bg': '#bf003f', 'fg': 'black'},
           'PAUSE': {'bg': '#196019', 'fg': 'white'},
           'LOOP': {'bg': '#000000', 'fg': 'white'}};

function gaussian_point(x, mew, sigma){
	var norm = new NormalDistribution(mew,sigma);
	return norm.getQuantile(x);
}

function instr_info_html(data){
	return Mustache.render(document.getElementById('instr-info-template').innerHTML, data);
}
function instr_info($this){
	console.log($this);
	if($this.attr('data-title')){
		$('.modal-body').html($this.attr('data-title'));
		$('.modal').modal({backdrop: false}).draggable({
			handle: ".modal-header"
		});
	}
}

function make_spacer(instrs){
	var len = 0;
	for(var i in instrs){
		instr = instrs[i];
		len += instr.length;
	}
	return [{
		'instr': {'instr':"&nbsp;", 'class':'noshadow'},
		'bg':'',
		'fg':'',
		'descr':'',
		'length':len}];
}

function straighten(data, stream, time_now, time_now_mean){
	var results;
	var clone;
	var len;
	if (data !== 'LOOP' && data.instr != 'PAUSE') {
		len = gaussian_point(percentile, data.length.mew, data.length.sigma);
		if(len < 0){
			return [{
			'instr': {'instr':"Negative Time"},
			'bg':'white',
			'fg':'black',
			'descr': '',
			'length': -1}];
		}
	}
	if(data.instr == 'PAUSE'){
		//contract the length of pauses as we expand the other instructions
		len = data.length - (time_now - time_now_mean);
		if(len < 0){
			return [{
			'instr': {'instr':"Deadline Missed"},
			'bg':'white',
			'fg':'black',
			'descr': '',
			'length': -1}];
		}
		var tmp = {'instr': {'instr':'PAUSE'}};
		stream.push({
			'instr': {'instr':'PAUSE'},
			'bg':legend.PAUSE.bg,
			'fg':legend.PAUSE.fg,
			'descr': instr_info_html({time_now: time_now, data: tmp, end:time_now+len, len:len}),
			'length': len * scale });
	}
	if(data == 'LOOP'){
		stream.push({
			'instr': {'instr':'LOOP'},
			'bg':legend.LOOP.bg,
			'fg':legend.LOOP.fg,
			'descr': '',
			'length': 50});
		results = [stream];
	} else {
		if(data.instr != 'PAUSE'){
			stream.push({
				'instr': data.instr,
				'bg':legend[data.instr.instr].bg,
				'fg':legend[data.instr.instr].fg,
				'descr': instr_info_html({time_now: time_now, data: data, end:time_now+len, len:len}),
				'length': len * scale });
		}
		if(data.children.length == 1){
			results = straighten(data.children[0], stream, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? data.length : data.length.mew));
		} else {
			results = [];
			var spacer = make_spacer(stream);
			for (var c = 0; c < data.children.length; c++){

				var res = straighten(data.children[c], c === 0 ? stream : spacer, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? data.length : data.length.mew));
				// pass it through if it's one, otherwise 
				if(res.length == 1){
					results.push(res[0]);
				} else {
					results.push.apply(results, res);
				}
			}
		}
	}
	return results;
}
function throttle(fn, threshhold, scope) {
  threshhold || (threshhold = 250);
  var last,
      deferTimer;
  return function () {
    var context = scope || this;

    var now = +new Date,
        args = arguments;
    if (last && now < last + threshhold) {
      // hold on to it
      clearTimeout(deferTimer);
      deferTimer = setTimeout(function () {
        last = now;
        fn.apply(context, args);
      }, threshhold);
    } else {
      last = now;
      fn.apply(context, args);
    }
  };
}

function render(){
	var res = [];
	for (var vm in cached_data){
		var data = cached_data[vm];
		streams = straighten(data, [], 0, 0);
		res.push({title: vm, data: streams});
	}
	$('#container').html(Mustache.render($('#template').html(), res));
	$('.stream').each(function(i, ele){
		$this = $(ele);
		// calculate the length of the container
		var maxlen = 0;
		$this.children().each(function(i, ele){
			maxlen += $(ele).width();
		});
		$this.css('width', maxlen *1.1);
		// add tooltips
		$this.find('.instr').tooltip({
			html: true
		}).click(function(){
			instr_info($(this));
		});
	});

	var leg = [];
	for(var instr in legend){
		leg.push({
			instr: instr,
			bg: legend[instr].bg,
			fg: legend[instr].fg,
			mean: lengths[instr].mew,
			std: lengths[instr].sigma,
			value: lengths[instr].mew == 'n/a' ? 'n/a' : gaussian_point(percentile, lengths[instr].mew, lengths[instr].sigma)
		});
	}
	$('#legend').html(Mustache.render($('#legend-template').html(), leg));
}
//this global variable is used to suppress firing the change event when keyup triggers a change in value in the slider
var hack = false;
function receive_data(data){
	cached_data = data;
	render();
	$('#perc').keyup(function(){
		var num = Number(this.value);
		if(!isNaN(num) && percentile !== num){
			percentile = num;
			hack = true;
			$('#percentile').slider('setValue', num);
			hack = false;
			render();
		}
	});
	$('#percentile').slider({
		max: 0.99,
		min: 0.01,
		value: 0.5,
        step: 0.01
    }).on('slide', throttle(function(ev){
		if(hack) return;
		var num = Number(ev.value);
		if(!isNaN(num && percentile !== num)){
			percentile = num;
			$('#perc').val(num);
			render();
		}
	}, 200, this));
	$('#zoom').keyup(function(){
		var num = Number(this.value);
		if(!isNaN(num) && scale !== num){
			scale = num;
			render();
		}
	});
	$('#zoomin').click(function(e){
		scale *= 1.1;
		$('#zoom').val(scale);
		render();
		e.preventDefault();
	});
	$('#zoomout').click(function(e){
		scale /= 1.1;
		$('#zoom').val(scale);
		render();
		e.preventDefault();
	});
}