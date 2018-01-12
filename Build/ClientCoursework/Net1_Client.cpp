/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!" 
if it successfully connects. The server, will continually broadcast a packet containing a 
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to 
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on 
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>




const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

Vector3 prev_position =   Vector3(100,100,100);
bool sentStart = false;
bool secondTime = false;

//Coords to send for stand and end points 
int startx;
int starty;
int endx;
int endy;


bool chaseRadius = true;

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, avatar(NULL)
	, grid_position(NULL)
	, curr_avatar_pos(new Vector3(0,0,0))
	
{
}

void Net1_Client::OnInitializeScene()
{    

	Camera * camera = GraphicsPipeline::Instance()->GetCamera();
	camera->SetPosition(Vector3(0.45f, 4.5f, 0.47f));
	camera->SetPitch(-90);
	draw_path = false;

	grid_position = new Vector3(0, 0, 0);
	

	path_vec.clear();
	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}




}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	avatar = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}

//Generate the maze using the info recieved from server.
void Net1_Client::GenerateMaze(string dt) {
	if (maze) {
		this->RemoveGameObject(maze);
	}
	int maze_edges = maze_size*(maze_size - 1) * 2;
	printf("\t Server created maze! Now rendering.. Number of edges = %d \n", maze_edges);
	bool * walls = new bool[maze_edges];


	for (int i = 0; i < maze_edges; i++) {
		walls[i] = dt[i] == '1' ? true : false;
	}

	maze_scalar = Matrix4::Scale(Vector3(1, 5.0f / (float(maze_size)* 3.0f), 1));
	maze = new MazeRenderer(walls, maze_size);
	maze->Render()->SetTransform(maze_scalar);
	this->AddGameObject(maze);

	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[to_string(i)]) {
			this->RemoveGameObject(enemies[to_string(i)]);
		}
	}
	for (int i = 0; i < num_enem; i++) {
		enemies[to_string(i)] = CommonUtils::BuildSphereObject("avatar",
			Vector3(0, 0.0f, 0),								//Position
			0.05f,									//Half dimensions
			false,									//Has Physics Object
			0.5f,									//Infinite Mass
			false,									//Has Collision Shape
			false,									//Dragable by the user
			Vector4(i*0.5f, 0.0f, 1.0f, 1.0f));	//Color 
		this->AddGameObject(enemies[to_string(i)]);
	}
	CreateGround(maze_size, maze->GetHalfDims());
}


//Create clickable ground 
void Net1_Client::CreateGround(int maze_sz, Vector3 halfdims) {
	
	for (int i = 0; i < maze_sz; i++) {
		for (int j = 0; j < maze_sz; j++) {
			Vector3 grid_pos = Vector3(halfdims.x*0.5f, 0.0f, halfdims.z*0.5f) + Vector3(i*halfdims.x*1.5f, 0.0f, j*halfdims.z*1.5f);
			this->AddGameObject(CommonUtils::BuildMazeNode(to_string(i)+" "+to_string(j),
				grid_pos,	                            //Position leading to 0.25 meter overlap on faces, and more on diagonals
				Vector3(halfdims.x,0.005f,halfdims.z),				//Half dimensions
				grid_position,									
				0.0f,									//Infinite Mass
				false,									//Has Collision Shape
				true,									//selectable by the user
				Vector4(0.5f, 0.8f, 0.0f, 1.0f)));
		}
	}
}

void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);


	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);



	//Send Init maze instructions to the server (maze size and density)
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_E)) {
		string data = "CHSE \n";
		chaseRadius = !chaseRadius;
		SendDataToServer(data);
	}


	//Send Init maze instructions to the server (maze size and density)
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_I)) {
			int maze_sz;
			float dens;
			int enem;
			printf("Choose maze size: \n");
			cin >> maze_sz;
			printf("Choose density: \n");
			cin >> dens;
			printf("Choose number of enemies: \n");
			cin >> enem;
			string data = "INIT " + to_string(maze_sz) + " " + to_string(dens).substr(0, 4) + " " + to_string(enem) + "\n";
			SendDataToServer(data);
	}

	//Send start end coords with left double click to the server
	if (Window::GetMouse()->DoubleClicked(MOUSE_LEFT)&& !(prev_position == *grid_position)) {
		*grid_position = Vector3((int)(grid_position->x * maze_size), (int)(grid_position->y * maze_size), (int)(grid_position->z * maze_size));
		Sleep(120);
			if (!sentStart) {
				startx = grid_position->x;
				starty = grid_position->z;
				sentStart = true;
			}
			else {
				endx = grid_position->x;
				endy = grid_position->z;
				
				//send start point for second time, so it avatar will continue from where it was
				if (!secondTime)	start = new Vector3(startx, starty, 0);
				else {
					start = curr_avatar_pos;
				}
				end = new Vector3(endx, endy, 0);
				secondTime = true;
				string send = "CRDS " + to_string(start->x) + " " + to_string(start->y) + " " + to_string(0) + " "
					+ to_string(end->x) + " " + to_string(end->y) + " " + to_string(0);
				SendDataToServer(&send[0]);
			}
			prev_position = *grid_position;
	}
	

	if (draw_path) {
		maze->DrawRoute(path_vec, 0.03f, maze_size);
	//	maze->DrawStartEndNodes(start, end);
	}

		

	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	//NCLDebug::DrawTextWs(box->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.f, 0.f, 1.f),
		//"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, serverConnection->address.port);

	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
	NCLDebug::AddStatusEntry(status_color, "    EvilBall Chase Mode: %s", chaseRadius ? "Radius" : "Line of Sight");
	NCLDebug::AddStatusEntry(status_color, "	Press I to recreate the maze");
	NCLDebug::AddStatusEntry(status_color, "	Press E to change AI chase mode");
	NCLDebug::AddStatusEntry(status_color, "    Double click to move (The first time 2 points are required)");
}


void Net1_Client::SendDataToServer(string data) {

	ENetPacket* packet = enet_packet_create(&data[0], data.length(), 0);
	enet_peer_send(serverConnection, 0, packet);
}

//Convert maze coords to world coords.
Vector3 Net1_Client::ConvertToWorldPos(Vector3 cellpos, int maze_sz, GameObject* obj) {
	float grid_scalar = 1.0f / (float)maze_size;
	Matrix4 transform = obj->Render()->GetWorldTransform();
	return transform * Vector3(
		(cellpos.x + 0.5f) * grid_scalar,
		0.1f,
		(cellpos.y + 0.5f) * grid_scalar) ;
}



void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
	//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
		{
			if (evnt.peer == serverConnection)
			{
				NCLDebug::Log(status_color3, "Network: Successfully connected to server!");
				
			}	
		}
		break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
		{   //printf("\t Received data from server %d. Data length: %d \n", evnt.peer->incomingPeerID, evnt.packet->dataLength);
			string id = extractId(evnt.packet->data);
			string data = extractData(evnt.packet->data, evnt.packet->dataLength);
		    
			// Receive maze size and density from server to create the maze . Respond "OOKK" so the server can send the actual walls ( bool );
			if (id == "INIT") 
			{
				stringstream ss;
				string vals;
				ss << data;
				ss >> vals;
				maze_size = stoi(vals);
				ss >> vals;
				maze_density = stof(vals);
				ss >> vals;
				num_enem = stoi(vals);
				printf("\t Received size: %d, density %f and enemies %d from server \n", maze_size, maze_density, num_enem);
				string response = "OOKK";
				SendDataToServer(response);
			}
			//Receive walls to build maze.
			else if (id == "MAZE") {
				GenerateMaze(data);
			}

			// Receive receive path coords to create and draw the final path.
			else if (id == "ROUT") {

				path_vec.clear();
				draw_path = true;
				stringstream ss;
				ss << data;
				string client_id;
				ss >> client_id;
				
				if (client_id == to_string(evnt.peer->outgoingPeerID)) {
					while (!ss.eof()) {
						string p;
						float x, y, z;
						ss >> p;
						if (p != "") {
							x = stof(p);
							ss >> p; y = stof(p);
							ss >> p; z = stof(p);
							Vector3 node_pos = Vector3(x, y, z);
							path_vec.push_back(node_pos);
						}
					}
				
				}
			
			}

			//Receive position of all the players
			else if (id == "POSI") {
				stringstream ss;
				ss << data;
				while (!ss.eof()) {
					string p;
					float x, y, z;
					ss >> p;
					string client_id = p;
					ss >> p;
					if (p != "") {
						x = stof(p);
						ss >> p; y = stof(p);
						ss >> p; z = stof(p);
						Vector3 avatar_pos = Vector3(x, y, z);
						if (client_id == to_string(evnt.peer->outgoingPeerID))
							*curr_avatar_pos = avatar_pos;
						(*avatars[client_id]->Render()->GetChildIteratorStart())->SetTransform(Matrix4::Translation(ConvertToWorldPos(avatar_pos, maze_size, avatars[client_id]))*Matrix4::Scale(Vector3(0.02f, 0.02f, 0.02f)));
					}
				}
			}
			//Receive position of all the Enemies
			else if (id == "ENEM") {
				if (num_enem > 0) {
					stringstream ss;
					ss << data;
					while (!ss.eof()) {
						string p;
						float x, y, z;
						ss >> p;
						string enemy_id = p;
						ss >> p;
						if (p != "") {
							bool isAggresive = stoi(p);
							ss >> p;
							x = stof(p);
							ss >> p; y = stof(p);
							ss >> p; z = stof(p);
							Vector3 avatar_pos = Vector3(x, y, z);
							if(isAggresive)  (*enemies[enemy_id]->Render()->GetChildIteratorStart())->SetTransform(Matrix4::Translation(ConvertToWorldPos(avatar_pos, maze_size, enemies[enemy_id]))*Matrix4::Scale(Vector3(0.04f, 0.04f, 0.04f)));
							else (*enemies[enemy_id]->Render()->GetChildIteratorStart())->SetTransform(Matrix4::Translation(ConvertToWorldPos(avatar_pos, maze_size, enemies[enemy_id]))*Matrix4::Scale(Vector3(0.02f, 0.02f, 0.02f)));
						}
					}
				}
			}
			//Connect tou server and receive the current number of clients connected to render them when maze is created;
			else if (id == "CONN") {
				
				stringstream ss;
				ss << data;
				string new_client_id;
				ss >> new_client_id;
				string clients_connected;
				ss >> clients_connected;
				for (int i = 0; i < stoi(clients_connected); i++) {
					if (avatars[to_string(i)]) {
						this->RemoveGameObject(avatars[to_string(i)]);
					}
					avatars[to_string(i)] = CommonUtils::BuildCuboidObject("avatar",
						Vector3(0, 0, 0),								//Position
						Vector3(0.05f, 0.05f, 0.05f),				//Half dimensions
						false,									//Has Physics Object
						0.5f,									//Infinite Mass
						false,									//Has Collision Shape
						false,									//Dragable by the user
						Vector4(i*0.5f, 0.0f, 1.0f, 1.0f));	//Color 
					this->AddGameObject(avatars[to_string(i)]);
				}
			}
			else
			{   	
				NCLERROR("Recieved Invalid Network Packet!");
				std::cout << "Length: " << evnt.packet->dataLength << endl;
			}

		}
		break;


	//Server has disconnected. Attempt to reconnect
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log(status_color3, "Network: Server has disconnected!");
			serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
			NCLDebug::Log("Network: Attempting to connect to server.");
		}
		break;
	}
}