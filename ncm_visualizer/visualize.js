var scale = 0.05;
var cached_data;

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
function receive_data(data){
	cached_data = data;
	function flatten(data, stream, time_now){
		var results;
		var clone;
		if(data.instr == 'PAUSE'){
			stream.push({
				'instr': {'instr':'PAUSE'}, 
				'descr': instr_info_html({time_now: time_now, data: data, end:time_now+data.length}), 
				'length': data.length * scale -2/*for the broders*/});
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
					'descr': instr_info_html({time_now: time_now, data: data, end:time_now+data.length}),
					'length': data.length * scale -2/*for the broders*/});
			}
			if(data.children.length == 1){
				results = flatten(data.children[0], stream, time_now+data.length);
			} else {
				results = [];
				for (var c = 0; c < data.children.length; c++){
					clone = JSON.parse(JSON.stringify(stream));
					results.push(flatten(data.children[c], clone, time_now+data.length));
				}
			}
		}
		return results;
	}
	streams = flatten(data, [], 0);
	/*
	for (var s in streams){
		var stream = streams[s];
		console.log('stream '+s);
		for (var d in stream){
			var drop = stream[d];
			console.log(drop);
		}
	}*/
	document.getElementById('container').innerHTML = Mustache.render(document.getElementById('template').innerHTML, streams);
	$( document ).tooltip({
		items: '.instr',
		// support html tooltips
		content:function(){
          return $(this).attr("data-title");
		}
	});
}
$('#zoomin').click(function(e){
	scale *= 1.1;
	receive_data(cached_data);
	e.preventDefault();
});
$('#zoomout').click(function(e){
	scale /= 1.1;
	receive_data(cached_data);
	e.preventDefault();
});