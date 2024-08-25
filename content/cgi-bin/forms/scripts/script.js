const myForm = document.getElementById('myForm');

myForm.addEventListener('submit', (e) => {
	e.preventDefault();
	const [name, email, msg, language, methode] = [
		document.getElementById('name').value,
		document.getElementById('email').value,
		document.getElementById('message').value,
		document.querySelector('input[name="lang"]:checked').value,
		document.querySelector('input[name="methode"]:checked').value
	];

	
})