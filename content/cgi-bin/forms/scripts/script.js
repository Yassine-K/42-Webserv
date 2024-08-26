const myCGI = document.getElementById('myForm');

myCGI.addEventListener('submit', (e) => {
	e.preventDefault();
	const myEntries = {}
	// const [name, email, msg, language, methode] = [
	// 	document.getElementById('name').value,
	// 	document.getElementById('email').value,
	// 	document.getElementById('message').value,
	// 	document.querySelector('input[name="lang"]:checked').value,
	// 	document.querySelector('input[name="methode"]:checked').value
	// ];
	const data = new FormData(myCGI)
	data.forEach((value, key) => {
		myEntries[key] = value
	});
	myCGI.action = '/cgi-bin/test.' + myEntries.language;
	myCGI.method = myEntries.methode;
	myCGI.submit();
})