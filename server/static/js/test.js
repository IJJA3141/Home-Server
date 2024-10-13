(async () => {
  fetch("/user", {
    method: "GET",
  })
    .then((_res) => {
      console.log(_res);
      return _res.json();
    })
    .then((_res) => {
      console.log(_res);
      if (_res.logged) {
        document.getElementById("test").innerText = "you are logged in as " + _res.username;
      } else {
        document.getElementById("test").innerText = "you are not logged in";
      }
    });
})();
