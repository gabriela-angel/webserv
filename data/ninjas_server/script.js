const fast = document.getElementById("fast");
const silent = document.getElementById("silent");
const deadly = document.getElementById("deadly");

if (fast) {
    fast.addEventListener('click', () => changeRoute("./fast/"))
}
if (silent) {
    silent.addEventListener('click', () => changeRoute("./silent/"))
}
if (deadly) {
    deadly.addEventListener('click', () => changeRoute("./deadly/"))
}

function changeRoute(route){
    window.location.href = route;
}