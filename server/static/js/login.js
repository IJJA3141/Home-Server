document.getElementById("submit-button").onclick = () => {
    str =
        document.getElementById("user").value+
        ":" +
        document.getElementById("password").value;

    str = btoa(str);
    console.log(str);

    fetch("/login", {
        method: "GET",
        headers: {
            Authorization: "Basic " + str,
        },
    });
};
