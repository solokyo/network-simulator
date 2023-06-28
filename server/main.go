package main

import (
	"jichen/network_simulator/server/internal/server"
	"log"
)

func main() {
	log.Println("Starting listening on port 8080")
	srv := server.NewHTTPServer(":8080")
	log.Fatal(srv.ListenAndServe())
}
