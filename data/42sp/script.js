const cadet = document.getElementById("cadet");
const uploads = document.getElementById("cadets_uploads");
const list = document.getElementById("cadets_list");

if (cadet) {
    cadet.addEventListener('click', () => changeRoute("/cadet/"));
}

if (uploads) {
    console.log("uploads")
    uploads.addEventListener('click', () => changeRoute("/cadets_uploads"));
}

if (list) {
    list.addEventListener('click', () => changeRoute("/cadets_list"));
}

function changeRoute(route){
    window.location.href = route;
}