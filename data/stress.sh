#Set variavel de ambiente
URL="http://localhost:8090"

# Requisições sequenciais:
for i in {1..1000}; do
  curl -s $URL >> /tmp/sequential_requests
done

# Requisições Simultâneas
for i in {1..100}; do
  curl -s $URL >> /tmp/simultaneous_requests &
done
wait

# Explodir número de conexões
for i in {1..500}; do
  curl -s $URL >> /tmp/connection_burst &
done
wait

# Connection Keep Alive
curl $URL $URL

# Connection close
curl -H "Connection: close"  $URL $URL

# Headers enormes (stress de parsing)
curl -H "X-Test: $(printf 'A%.0s' {1..8000})" $URL


# Métodos não implementados
curl -X PUT $URL
curl -X PATCH $URL

# Uploads grandes
dd if=/dev/zero bs=1M count=50 | curl -X POST --data-binary @- $URL

# Loop infinito
while true; do curl -s $URL >> /tmp/infinite_loop; done