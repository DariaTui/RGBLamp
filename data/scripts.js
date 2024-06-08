$(document).ready(function(){
	$('.button').click(function(){
		var b = $(this).attr('id');
		var url = '/' + b;
		$.get(url,
		function(){
			$('#' + b).fadeOut(200);
			$('#' + b).fadeIn(200);
		});
	});
});