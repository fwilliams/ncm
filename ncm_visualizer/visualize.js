var scale = 0.05;
var percentile = 0.50;
var cached_data;

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


function flatten(data, stream, time_now, time_now_mean){
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
			'descr': '', 
			'length': -1}];
		}
		var tmp = {'instr': {'instr':'PAUSE'}, 'length': len};
		stream.push({
			'instr': {'instr':'PAUSE'}, 
			'descr': instr_info_html({time_now: time_now, data: tmp, end:time_now+len, len:len}), 
			'length': len * scale -2/*for the broders*/});
		results = stream;
	}
	if(data == 'LOOP'){
		stream.push({
			'instr': {'instr':'LOOP'}, 
			'descr': '', 
			'length': -1});
		results = stream;
	} else {
		if(data.instr != 'PAUSE'){
			stream.push({
				'instr': data.instr, 
				'descr': instr_info_html({time_now: time_now, data: data, end:time_now+len, len:len}),
				'length': len * scale -2/*for the broders*/});
		}
		if(data.children.length == 1){
			results = flatten(data.children[0], stream, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? len : data.length.mew));
		} else {
			results = [];
			for (var c = 0; c < data.children.length; c++){
				clone = JSON.parse(JSON.stringify(stream));
				results.push(flatten(data.children[c], clone, time_now+len, time_now_mean+(data.instr == 'PAUSE' ? len : data.length.mew)));
			}
		}
	}
	return results;
}

function render(){
	streams = flatten(cached_data, [], 0, 0);
	$('#container').html(Mustache.render($('#template').html(), streams));
	$('#container').tooltip({
		items: '.instr',
		// support html tooltips
		content:function(){
          return $(this).attr("data-title");
		}
	});
}
//this global variable is used to suppress firing the change event when keyup triggers a change in value in the slider
var hack = false;
function receive_data(data){
	cached_data = data;
	render();
	$('#perc').keyup(function(event){
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
}
$('#zoomin').click(function(e){
	scale *= 1.1;
	render();
	e.preventDefault();
});
$('#zoomout').click(function(e){
	scale /= 1.1;
	render();
	e.preventDefault();
});