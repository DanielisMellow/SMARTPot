function toggleCheckbox(element) {
    var xhr = new XMLHttpRequest();
    if (element.checked) {
      xhr.open("GET", "/update?output=" + element.id + "&state=1", true);
    } else {
      xhr.open("GET", "/update?output=" + element.id + "&state=0", true);
    }
    xhr.send();
  }
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature").innerHTML =
          this.responseText;
      }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
  }, 10000);

  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("humidity").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
  }, 10000);
