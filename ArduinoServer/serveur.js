// Chargement de socket.io
var WebSocketServer = require('websocket').server;

// mqtt listener
var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://broker.hivemq.com')


var http = require('http');
var fs = require('fs');
var path = require("path");
var url = require("url");


function sendError(errCode, errString, response) {
  response.writeHead(errCode, {"Content-Type": "text/plain"});
  response.write(errString + "\n");
  response.end();
  return;
}

function sendFile(err, file, response) {
  if(err) return sendError(500, err, response);
  response.writeHead(200);
  response.write(file, "binary");
  response.end();
}

function getFile(exists, response, localpath) {
  if(!exists) return sendError(404, '404 Not Found', response);
  fs.readFile(localpath, "binary",
   function(err, file){ sendFile(err, file, response);});
}

function getFilename(request, response) {
  var urlpath = url.parse(request.url).pathname; 
  var localpath = path.join(process.cwd(), urlpath); 
  fs.exists(localpath, function(result) { getFile(result, response, localpath)});
}

// Creation du serveur
var server = http.createServer(getFilename);

client.on('connect', function () {
  client.subscribe('miage1/menez/sensors/temp', error());
  client.subscribe('miage1/menez/sensors/led', error());
  
})

client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())
})

wsServer = new WebSocketServer({
    httpServer: server
});

console.log("Serveur crée");
var connections = [];
// WebSocket server
wsServer.on('request', function (request) {
    var connection = request.accept(null, request.origin);
	console.log("connection Reussi");
    // This is the most important callback for us, we'll handle
    // all messages from users here.
    connection.on('message', function (message, connection=this) {
            if (message.type === 'utf8') {
				console.log(message);
                // process WebSocket message
                if (message.utf8Data === "Arduino") {
					console.log("Arduino");
                    connections[0] = connection;
                }
                if (message.utf8Data === 'Web') {
					console.log("Site Web");
                    connections[1] = connection;
                }
                
            }

            if (connection === connections[0] && connections[1] !== undefined) {
				console.log("Web to ESP : ");
                connections[1].send(message.utf8Data);

            }
            if (connection === connections[1] && connections[0] !== undefined) {
				console.log("ESP to WEB : ");
                connections[0].send(message.utf8Data);
            }
        }
    );


});

function error (err){
    if (err) {
      console.log("oups problème de connexion broker");
    }
	else{
		console.log("en place !");
	}
}
  





server.listen(8080);
