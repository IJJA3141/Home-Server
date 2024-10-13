document.getElementById("submit-button").onclick = () => {
  str =
    document.getElementById("user").value +
    ":" +
    document.getElementById("password").value;

  str = btoa(str);
  console.log(str);

  fetch("/login", {
    method: "GET",
    headers: {
      Authorization: "Basic " + str,
    },
  }).then((_res) => {
    if (_res.status == 200) {
      const queryString = window.location.search;
      const urlParams = new URLSearchParams(queryString);
      const next = urlParams.get("continue");
      window.location.href = next;
    } else if (_res.status == 407) {
      _res.text().then((_text) => {
        document.getElementById("testlogin").innerText =
          "login failed " + _text;
      });
    }
  });
};
