
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\PhysicsNode.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "EvilBall.h"
#include <sstream>
#include "MazePlayer.h"

//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
float rotation = 0.0f;

MazeGenerator	generator;
int maze_edges = 0;
bool* edges;
int maze_size = 0;
float maze_density = 0.0f;

int clients_connected = 0;

EvilBall *b1 = NULL;
vector<EvilBall*> enemies;
int num_enem = 0;

vector<Player*> players_v;



std::map<ENetPeer*, Player*> players;
std::map<ENetPeer*, Player*>::iterator players_it;


void Win32_PrintAllAdapterIPAddresses();

int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}

void BroadcastData(char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data), 0);
	enet_host_broadcast(server.m_pNetwork, 0, packet);
}

void SendDataToClient(string data, ENetPeer* peer) {
	ENetPacket* packet = enet_packet_create(&data[0], data.length(), 0);
	enet_peer_send(peer, 0, packet);
}

string extractId(enet_uint8* packet) {
	stringstream ss;
	for (int i = 0; i < 4; i++) {
		ss << packet[i];
	}
	string res;
	ss >> res;
	return res;
}

string extractData(enet_uint8* packet, int length) {
	if (length > 4) {
		stringstream ss;

		for (int i = 4; i < length; i++) {
			ss << packet[i];
		}
		string res;
		getline(ss, res);
		return res.substr(1);
	}
	else return "";
}

//Calculate and save the velocities of the final path.
void calculate_velocities(ENetPeer* peer) {
	for (int i = 0; i < players[peer]->avatar_cellpos.size(); i++) {
	
		Vector3 startp = players[peer]->avatar_cellpos[i];
	
		if ( i < players[peer]->avatar_cellpos.size()-1) {
			Vector3 endp = players[peer]->avatar_cellpos[i+1];
			Vector3 veloc = endp - startp;
			if (abs(veloc.x) < 1 && abs(veloc.y) == 0) {
				if (veloc.x < 0) veloc.x == -1;
				else veloc.x = 1;
			}
			else if (abs(veloc.y) < 1 && abs(veloc.x) == 0) {
				if (veloc.y < 0) veloc.y == -1;
				else veloc.y = 1;
			}
			players[peer]->avatar_velocities.push_back(veloc);
			
		}
		
	}

}

//Based on Damianos Gouzkouris implementation for smooth transition after giving new coordinates to the server.
void SmoothTransition(Player* player) {
	if (player->avatar_cellpos.size() > 1) {
		if (player->avatar_cellpos[0].x != player->avatar_cellpos[1].x) {
			if ((player->avatar->GetPosition().x < player->avatar_cellpos[0].x) == (player->avatar->GetPosition().x > player->avatar_cellpos[1].x)) {
				player->avatar_cellpos[0] = player->avatar->GetPosition();
			}
			else {
				player->avatar_cellpos.insert(player->avatar_cellpos.begin(), player->avatar->GetPosition());
			}
		}
		
		else if (player->avatar_cellpos[0].y != player->avatar_cellpos[1].y) {
			if ((player->avatar->GetPosition().y < player->avatar_cellpos[0].y) == (player->avatar->GetPosition().y > player->avatar_cellpos[1].y)) {
				player->avatar_cellpos[0] = player->avatar->GetPosition();
			}
			else {
				player->avatar_cellpos.insert(player->avatar_cellpos.begin(), player->avatar->GetPosition());
			}
		}
	}
}


// Update players positions based on velocities
void UpdatePlayerPositions() {
	players_v.clear();
	//Update position for all avatars;
	string send = "POSI ";
	for (players_it = players.begin(); players_it != players.end(); ++players_it) {
		if ((*players_it).second->enable_avatar) {
			if ((*players_it).second->avatarIndex < (*players_it).second->avatar_velocities.size()) {
				(*players_it).second->avatar->SetLinearVelocity((*players_it).second->avatar_velocities[(*players_it).second->avatarIndex] * 1.5f);
				float avatar_dist = ((*players_it).second->avatar->GetPosition() - (*players_it).second->avatar_cellpos[(*players_it).second->avatarIndex + 1]).Length();
				if (avatar_dist <= 0.1f) {
					(*players_it).second->avatar->SetPosition((*players_it).second->avatar_cellpos[(*players_it).second->avatarIndex + 1]);
					(*players_it).second->avatarIndex++;
				}
				send += (*players_it).second->peer_id + " " + to_string((*players_it).second->avatar->GetPosition().x) + " " +
					to_string((*players_it).second->avatar->GetPosition().y) + " " +
					to_string((*players_it).second->avatar->GetPosition().z) + " ";		
				//store all players current position
				(*players_it).second->cur_cell_pos = (*players_it).second->avatar_cellpos[(*players_it).second->avatarIndex];
			}
			else (*players_it).second->avatar->SetLinearVelocity(Vector3(0,0,0));

			
		}
		
		players_v.push_back((*players_it).second);
	}
	BroadcastData(&send[0]);
}

// Update Enemy positions and send them to clients (FSM with 2 states used)
void UpdateEnemyPositions() {
	if (enemies.size() > 0) {
		string send = "ENEM ";
		for (int i = 0; i < enemies.size(); i++) {
			if (enemies[i]->GetStateName() == "patrol") {
				enemies[i]->Patrol(players_v);
			}
			else enemies[i]->Chase();
			
			send += enemies[i]->GetPositionAsString();
		}
		BroadcastData(&send[0]);
	}
}

int main(int arcg, char** argv)
{   
	srand((uint)time(NULL));
	
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");

	
	//Initialise the PhysicsEngine
	PhysicsEngine::Instance();


	Win32_PrintAllAdapterIPAddresses();

	timer.GetTimedMS();
	while (true)
	{
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{   
			
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT: {
				//Create a player when connected
				printf("-%d Client Connected\n", evnt.peer->incomingPeerID);
				clients_connected++;
				Player *p = new Player();
				p->enable_avatar = false;
				players[evnt.peer] = p;
				players[evnt.peer]->avatar = new PhysicsNode();
				players[evnt.peer]->peer_id = to_string(evnt.peer->incomingPeerID);
				PhysicsEngine::Instance()->AddPhysicsObject(players[evnt.peer]->avatar);



				string send = "CONN " + to_string(evnt.peer->incomingPeerID) +" " + to_string(clients_connected);
				BroadcastData(&send[0]);
				break;
			}

			case ENET_EVENT_TYPE_RECEIVE: 
			{

				printf("\t Received data from client %d. Data length: %d \n", evnt.peer->incomingPeerID, evnt.packet->dataLength);
				string id = extractId(evnt.packet->data);
				string data = extractData(evnt.packet->data, evnt.packet->dataLength);
				string client_id = to_string(evnt.peer->incomingPeerID);
				//Receive maze size and density from client to generate and send back maze info.
				if (id == "INIT") {
				     
					for (int i = 0; i < enemies.size(); i++) {
						delete enemies[i];
					}
					enemies.clear();

					stringstream ss;
					string vals;
					ss << data;
					ss >> vals;
					maze_size = stoi(vals);
					ss >> vals;
					maze_density = stof(vals);
					ss >> vals;
					num_enem = stoi(vals);
					printf("\t Received maze size %d , density %f and enemies %d from client %d \n", maze_size, maze_density, num_enem, evnt.peer->incomingPeerID);
					string send = id +" "+data;
					BroadcastData(&send[0]);

					printf("\t Generating Maze... \n");
					generator.Generate(maze_size, maze_density);
					maze_edges = maze_size * (maze_size - 1) * 2;
					edges = new bool[maze_edges];
					for (int i = 0; i < maze_edges; i++) {
						edges[i] = (generator.allEdges[i]._iswall);
					}
					printf("\t Maze Generated!.\n");
					for (int i = 0; i < num_enem; i++) {
						EvilBall *b = new EvilBall(&generator, to_string(i));
						b->EnableBall();
						enemies.push_back(b);
					}

				}
				//All clients received maze size and density. Send them the maze info.
				else if (id == "OOKK") {
					printf("\t Sending data to Clients!\n");
					string send = "MAZE ";
					for (int i = 0; i < maze_edges; i++) {
						send += to_string(edges[i]);
					}
					BroadcastData(&send[0]);
				}
				//Change chase mode
				else if (id == "CHSE") {
					EvilBall::ToggleChase();
				}
				//Send the client the final path to draw and render (based on a star)
				else if (id == "CRDS") {
					printf("\t Received coords for start and end point: %s\n", data.c_str());
					SearchAStar* search_as = new SearchAStar();
					search_as->SetWeightings(1.0f, 1.0f);
					stringstream ss;
					int indexs, indexe;
					int x, y;
					string coord;
					ss << data;
					for (int i = 0; i < 2; i++) {
						ss >> coord; x = stoi(coord);
						ss >> coord; y = stoi(coord); ss >> coord;
						if (i == 0) indexs = y % maze_size * maze_size + x;
						else indexe = y % maze_size * maze_size + x;
					}

					GraphNode* start = &generator.allNodes[indexs];
					GraphNode* end = &generator.allNodes[indexe];

					bool has_velocity = false;
					if (players[evnt.peer]->avatar_cellpos.size() > 0) {
						has_velocity = true;
					}
					else {
						players[evnt.peer]->avatar->SetPosition(start->_pos);
					}
					players[evnt.peer]->enable_avatar = true;
					players[evnt.peer]->avatarIndex = 0;
					players[evnt.peer]->avatar_cellpos.clear();
					players[evnt.peer]->avatar_velocities.clear();
				//	players[evnt.peer]->avatar->SetPosition(start->_pos);
				

					search_as->FindBestPath(start, end);
					players[evnt.peer]->final_path = search_as->GetFinalPath();
					string send = "ROUT " + to_string(evnt.peer->incomingPeerID) + " ";

					
					for (std::list<const GraphNode*>::iterator it = players[evnt.peer]->final_path.begin(); it != players[evnt.peer]->final_path.end(); it++) {
						send += to_string((*it)->_pos.x) + " " + to_string((*it)->_pos.y) + " " + to_string((*it)->_pos.z) + " ";
						players[evnt.peer]->avatar_cellpos.push_back((*it)->_pos);
					}
					printf("\t Broadcasting final graph to client.");
				
					SendDataToClient(&send[0], evnt.peer);
					if (has_velocity) {
						SmoothTransition(players[evnt.peer]);
					}
					calculate_velocities(evnt.peer);
					
				}
				else
				{
					NCLERROR("Size of package received: %d", evnt.packet->dataLength);
					NCLERROR("Recieved Invalid Network Packet!");
				}
				enet_packet_destroy(evnt.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				clients_connected--;
				break;
			}
		});

		if (accum_time >= UPDATE_TIMESTEP) {
			accum_time = 0.0f;
			UpdatePlayerPositions();
			UpdateEnemyPositions();	
		}
		PhysicsEngine::Instance()->Update(dt);

		Sleep(0);
	}

	system("pause");
	server.Release();
}




//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}