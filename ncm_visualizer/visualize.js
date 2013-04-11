function receive_data(data){
	var res = [];
	function flatten(data, index){
		res[index] = res[index] || [];
		if(data == 'LOOP'){
			res[index].push('LOOP');
		} else {
			res[index].push(data.instr);
		}
		for (var c in data.children){
			flatten(data.children[c], index+Number(c));
		}
	}
	console.log(data);
	console.log(flatten(data, 0));
	console.log(res);
	document.getElementById('container').innerHTML = Mustache.render('test {{a}}', {'a':'b'});
}