document.getElementById("submit-button").onclick = () => {
    str =
        document.getElementById("user").innerText +
        ":" +
        document.getElementById("password").innerText;

    str = btoa(str);
    console.log(str);

    fetch("/login", {
        method: "GET",
        headers: {
            Authorization: "Basic " + str,
        },
    });
};
