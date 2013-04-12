var scale = 0.2;
var percentile = 0.50;
var cached_data;


legend = {'FUTURE': {'bg': 'white', 'fg': 'black'},
           'HALT': {'bg': 'cyan', 'fg': 'black'},
           'IF': {'bg': 'white', 'fg': 'black'},
           'MODE': {'bg': 'white', 'fg': 'black'},
           'CREATE': {'bg': 'white', 'fg': 'black'},
           'DESTROY': {'bg': 'white', 'fg': 'black'},
           'SEND': {'bg': 'white', 'fg': 'black'},
           'RECEIVE': {'bg': 'white', 'fg': 'black'},
           'SYNC': {'bg': 'white', 'fg': 'black'},
           'HANDLE': {'bg': 'white', 'fg': 'black'},
           'NOP': {'bg': 'white', 'fg': 'black'},
           'SET_COUNTER': {'bg': 'white', 'fg': 'black'},
           'ADD_TO_COUNTER': {'bg': 'white', 'fg': 'black'},
           'SUB_FROM_COUNTER': {'bg': 'white', 'fg': 'black'},
           'LOOP': {'bg': 'black', 'fg': 'white'},
           'PAUSE': {'bg': 'white', 'fg': 'black'}};

function gaussian_point(x, mew, sigma){
	var norm = new NormalDistribution(mew,sigma);
	return norm.getQuantile(x);
}

function instr_info_html(data){
	return Mustache.render(document.getElementById('instr-info-template').innerHTML, data);
}
function instr_info(t){
	$this = $(t);
	if($this.attr('data-title')){
		$('#dialog').html($this.attr('data-title'));
		$('#dialog').dialog();
	}
}


function straighten(data, stream, time_now, time_now_mean){
	var results;
	var clone;
	var len;
	if (data !== 'LOOP' && data.instr != 'PAUSE') {
		len = gaussian_point(percentile, data.length.mew, data.length.sigma);
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
			'length': len * scale -2/*for the broders*/});
		results = stream;
	}
	if(data == 'LOOP'){
		stream.push({
			'instr': {'instr':'LOOP'}, 
			'bg':legend.LOOP.bg,
			'fg':legend.LOOP.fg,
			'descr': '', 
			'length': 50});
		results = stream;
	} else {
		if(data.instr != 'PAUSE'){
			stream.push({
				'instr': data.instr,
				'bg':legend[data.instr.instr].bg,
				'fg':legend[data.instr.instr].fg,
				'descr': instr_info_html({time_now: time_now, data: data, end:time_now+len, len:len}),
				'length': len * scale -2/*for the broders*/});
		}
		if(data.children.length == 1){
			results = straighten(data.children[0], stream, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? data.length : data.length.mew));
		} else {
			results = [];
			for (var c = 0; c < data.children.length; c++){
				clone = JSON.parse(JSON.stringify(stream));
				results.push(straighten(data.children[c], clone, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? data.length : data.length.mew)));
			}
		}
	}
	return results;
}

function render(){
	streams = straighten(cached_data, [], 0, 0);
	// calculate the length of the container
	var maxlen = 0;
	for (var i in streams){
		var tot = 0;
		for (var j in streams[i]){
			tot += streams[i][j].length+2;
		}
		maxlen = Math.max(maxlen, tot);
	}
	$('#container').css('width', maxlen);
	console.log(streams);
	$('#container').html(Mustache.render($('#template').html(), streams));
	$('#container').tooltip({
		items: '.instr',
		// support html tooltips
		content:function(){
          return $(this).attr("data-title");
		}
	});
	var leg = [];
	for(var instr in legend){
		leg.push({
			instr: instr,
			bg: legend[instr].bg,
			fg: legend[instr].fg
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
			$('#percentile').slider('value', num);
			hack = false;
			render();
		}
	});
	$('#percentile').slider({
		max: 0.99,
		min: 0.01,
		value: 0.5,
        step: 0.01,
		change: function(event, ui){
			if(hack) return;
			var num = Number(ui.value);
			if(!isNaN(num && percentile !== num)){
				percentile = num;
				$('#perc').val(num);
				render();
			}
		}
	});
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