## Copyright

**Name:** Vișănescu Bogdan-Emilian  
**Group:** 323 CA  
# TCP/UDP Server with Subscription Management and Wildcards

This project implements a publish-subscribe system consisting of a server and TCP clients. The server manages TCP and UDP connections, allowing clients to subscribe to topics and receive messages according to their subscriptions. The system supports wildcards for subscriptions, enabling high flexibility in topic management.

---

## Main Features

1. **TCP and UDP Connection Management**:
   - The server listens on two sockets: one TCP for client communication and one UDP for receiving messages from other sources.

2. **Subscription and Unsubscription to Topics**:
   - Clients can subscribe to specific topics or use wildcards (`+` and `*`).
   - Clients can unsubscribe from topics, and the server updates the subscription list accordingly.

3. **Topic Wildcards**:
   - `+`: Matches exactly one segment of a topic.
   - `*`: Matches zero or more segments.

4. **Subscription Persistence**:
   - Client subscriptions are preserved even if they disconnect and later reconnect.

5. **Message Delivery**:
   - Messages received on the UDP socket are forwarded to all clients subscribed to the corresponding topics.

---

## Project Structure

### Main Files

1. **`server.cpp`**:
   - Contains the server implementation, including connection management, subscriptions, and message forwarding.

2. **`tcp_client.cpp`**:
   - Implements the TCP client that allows users to interact with the server.
   - Clients can send commands such as `subscribe`, `unsubscribe`, and `exit`.

3. **`utils.cpp`**:
   - Contains helper functions used by both the server and the client for sending and receiving complete data over sockets.

4. **`server.hpp`**:
   - Declarations of functions and structures used in the server.

5. **`tcp_client.hpp`**:
   - Declarations of functions and structures used in the client.

6. **`utils.hpp`**:
   - Declarations of helper functions from `utils.cpp`.

---

## Important Functions in `server.cpp`

### 1. `setup_server`
This function configures the TCP and UDP sockets and binds them to the specified port. It initializes the server so it can listen for connections and messages.

- **Main Steps**:
  1. Create a TCP socket using `socket(AF_INET, SOCK_STREAM, 0)`.
  2. Create a UDP socket using `socket(AF_INET, SOCK_DGRAM, 0)`.
  3. Configure the server address (`struct sockaddr_in`) to listen on all interfaces (`INADDR_ANY`) and the specified port.
  4. Bind both TCP and UDP sockets to the configured address using `bind`.
  5. Put the TCP socket in listening mode using `listen`.

- **Role**:
  - Prepares the server to accept TCP connections and receive UDP messages.

---

### 2. `handle_tcp`
This function handles new client connections on the TCP socket. It is called when the server detects activity on the TCP socket.

- **Main Steps**:
  1. Accept a new connection using `accept`.
  2. Configure the client socket to allow address reuse (`SO_REUSEADDR`) and disable Nagle’s algorithm (`TCP_NODELAY`).
  3. Add the client socket to the active file descriptor set (`FD_SET`) and update the maximum file descriptor value (`fd_max`).

- **Role**:
  - Allows the server to accept new client connections and manage them afterward.

---

### 3. `handle_udp`
This function receives messages on the UDP socket and forwards them to clients subscribed to the corresponding topics.

- **Main Steps**:
  1. Receive a UDP message using `recvfrom`.
  2. Extract the topic from the message and check if there are clients subscribed to matching topics.
  3. Use a `std::set` to avoid sending duplicate messages to the same client.
  4. Forward the message to each subscribed client using `send_all`.

- **Role**:
  - Ensures delivery of UDP messages to subscribed clients.

---

### 4. `handle_connect`
This function manages the connection of a new client or the reconnection of an existing client.

- **Main Steps**:
  1. Check if the client is already registered in the `clients` list.
  2. If it’s a new client, create a `client_data_t` structure to store client info and add it to the list.
  3. If the client is already connected, close the new connection and display a warning.
  4. If the client is disconnected, update its state and resume the connection.

- **Role**:
  - Handles both new and reconnecting clients.

---

### 5. `handle_subscribe`
This function adds a client to the list of subscribers for a given topic.

- **Main Steps**:
  1. Find the client in the `clients` list using its ID.
  2. Add the client to the subscriber list for the specified topic.

- **Role**:
  - Allows clients to subscribe to topics in order to receive relevant messages.

---

### 6. `handle_unsubscribe`
This function removes a client from the subscriber list for topics that match a given pattern.

- **Main Steps**:
  1. Iterate through all topics in `topics_to_clients`.
  2. Check if the current topic matches the specified pattern using `topic_matches`.
  3. Remove the client from the subscriber list of matching topics.

- **Role**:
  - Allows clients to unsubscribe from topics.

---

### 7. `handle_exit`
This function manages client disconnection by marking the client as disconnected.

- **Main Steps**:
  1. Find the client in the `clients` list using its ID.
  2. Mark the client as disconnected and close the connection.
  3. Remove the client socket from the active file descriptor set (`FD_CLR`).

- **Role**:
  - Ensures proper management of client disconnections.

---

### 8. `run_server`
This is the server’s main loop that manages socket and stdin events.

- **Main Steps**:
  1. Initialize the set of active file descriptors and wait for events using `select`.
  2. Check for activity on the TCP socket, UDP socket, or stdin.
  3. Call the corresponding functions (`handle_tcp`, `handle_udp`, `handle_clients`) to handle detected events.
  4. Allows the server to stop via the `exit` command entered in stdin.

- **Role**:
  - Coordinates all server activities, including connection management and message processing.

---

## Important Functions in `tcp_client.cpp`

### 1. `setup_connection`
This function sets up the TCP connection with the server. The client sends a `CONNECT` message to register with the server.

- **Main Steps**:
  1. Create a TCP socket and connect to the server using `connect`.
  2. Send a `CONNECT` message to the server to register.

- **Role**:
  - Allows the client to connect and register with the server.

---

### 2. `handle_server_message`
This function receives and processes messages sent by the server. Messages are displayed to the user.

- **Main Steps**:
  1. Receive messages from the server using `recv_all`.
  2. Process received messages and display them to the user.

- **Role**:
  - Ensures communication between server and client.

---

### 3. `handle_user_input`
This function handles commands entered by the user.

- **Supported Commands**:
  - `subscribe <topic>`: Subscribes the client to a topic.
  - `unsubscribe <topic>`: Unsubscribes the client from a topic.
  - `exit`: Closes the connection with the server.

- **Main Steps**:
  1. Read the command entered by the user.
  2. Send the command to the server for processing.

- **Role**:
  - Allows the user to interact with the server.

---

### 4. `run_tcp_client`
This is the client’s main loop that manages interaction with the server and the user.

- **Main Steps**:
  1. Wait for events from the server or the user.
  2. Process messages received from the server.
  3. Process commands entered by the user.

- **Role**:
  - Coordinates all client activities, including communication with the server and user interaction.

---

## Topic Wildcards

### `+` (Single-Segment Wildcard)
- Matches exactly one segment of a topic.
- Example:
  - Subscription: `upb/+/temperature`
  - Matching messages:
    - `upb/ec/temperature`
    - `upb/room/temperature`
  - Non-matching messages:
    - `upb/ec/room/temperature`

### `*` (Multi-Segment Wildcard)
- Matches zero or more segments.
- Example:
  - Subscription: `upb/*`
  - Matching messages:
    - `upb/ec/temperature`
    - `upb/ec/room/temperature`
    - `upb`

---

## Data Structures

### 1. `clients` (in `server.cpp`)
- Type: `std::map<std::string, client_data_t *>`
- Description: Stores client information indexed by client ID.

### 2. `topics_to_clients` (in `server.cpp`)
- Type: `std::map<std::string, std::vector<client_data_t *>>`
- Description: Stores the list of clients subscribed to each topic.

---

## Main Server Flow

1. **Initialization**:
   - The server sets up TCP and UDP sockets and starts listening for connections and messages.

2. **Connection Management**:
   - Clients connect via TCP, and the server manages their subscriptions.

3. **Message Reception**:
   - Messages received on the UDP socket are processed and forwarded to subscribed clients.

4. **Client Disconnection**:
   - Clients can disconnect, and the server preserves their subscriptions for potential reconnection.

5. **Server Shutdown**:
   - The server can be stopped via the `exit` command entered in stdin.

---

## Main Client Flow

1. **Initialization**:
   - The client sets up a TCP connection with the server.

2. **User Interaction**:
   - The client waits for user commands (`subscribe`, `unsubscribe`, `exit`).

3. **Message Reception**:
   - The client receives messages from the server and displays them to the user.

4. **Connection Closure**:
   - The client sends an `EXIT` message to the server and closes the connection.
