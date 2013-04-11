function receive_data(data){
	function flatten(data, stream){
		var results;
		var clone;
		if(data == 'LOOP'){
			stream.push({'instr':'LOOP'});
			results = stream;
		} else {
			stream.push(data.instr);
			if(data.children.length == 1){
				results = flatten(data.children[0], stream);
			} else {
				results = [];
				for (var c = 0; c < data.children.length; c++){
					clone = JSON.parse(JSON.stringify(stream));
					results.push(flatten(data.children[c], clone));
				}
			}
		}
		return results;
	}
	streams = flatten(data, [], 0);
	for (var s in streams){
		var stream = streams[s];
		console.log('stream '+s);
		for (var d in stream){
			var drop = stream[d];
			console.log(drop.instr);
		}
	}
	document.getElementById('container').innerHTML = Mustache.render('{{#.}}{{#.}}{{instr}},{{/.}}<br/>{{/.}}', streams);
}