# ================================================================================= #

#Set variavel de ambiente
URL="http://localhost:8084"

# location /
curl -X GET "$URL"

#location /silent
curl -X GET "$URL/silent"
curl -d "" "$URL/silent/silent.py"

#location /deadly
curl -i -c cookies.txt http://127.0.0.1:8084/deadly/deadly.go # Inicializa o cookie
curl -i -b cookies.txt -c cookies.txt -d "" http://127.0.0.1:8084/deadly/deadly.go
curl -i -b cookies.txt http://127.0.0.1:8084/deadly/deadly.go

#location /forbidden
curl -X GET "$URL/forbidden"

# ================================================================================= #

#Set variavel de ambiente
URL="http://localhost:8085"

# location /
curl -X GET "$URL"

#location /cadet
curl -X GET "$URL/cadet"
curl -X POST "$URL/cadet" -F "photo=@photo.jpg" # fazer upload
curl -X POST "$URL/cadet/cadet_create.py" -F "name=John Doe" -F "photo=@photo.jpg"

#location /cadets_uploads
curl -X GET "$URL/cadets_uploads"

#location /cadets_list
curl -X GET "$URL/cadets_list"
curl -X DELETE "$URL/cgi/cadet_delete.py?name=John+Doe&file=person.jpg"

# ================================================================================= #