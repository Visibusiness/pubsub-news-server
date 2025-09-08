## Copyright

**Nume:** Vișănescu Bogdan-Emilian  
**Grupă:** 323 CA  
# Server TCP/UDP cu Gestionare de Subscrieri și Wildcard-uri

Acest proiect implementează un sistem de tip publish-subscribe, format dintr-un server și clienți TCP. Serverul gestionează conexiuni TCP și UDP, permițând clienților să se aboneze la topicuri și să primească mesaje în funcție de subscripțiile lor. Sistemul suportă wildcard-uri pentru subscripții, ceea ce permite o flexibilitate ridicată în gestionarea topicurilor.

---

## Funcționalități principale

1. **Gestionarea conexiunilor TCP și UDP**:
   - Serverul ascultă pe două socket-uri: unul TCP pentru comunicarea cu clienții și unul UDP pentru primirea mesajelor de la alte surse.

2. **Subscriere și dezabonare la topicuri**:
   - Clienții se pot abona la topicuri specifice sau folosind wildcard-uri (`+` și `*`).
   - Clienții se pot dezabona de la topicuri, iar serverul actualizează lista de subscripții în consecință.

3. **Wildcard-uri pentru topicuri**:
   - `+`: Se potrivește cu exact un segment al unui topic.
   - `*`: Se potrivește cu zero sau mai multe segmente.

4. **Persistența subscripțiilor**:
   - Subscripțiile clienților sunt păstrate chiar dacă aceștia se deconectează și se reconectează ulterior.

5. **Trimiterea mesajelor**:
   - Mesajele primite pe socket-ul UDP sunt trimise către toți clienții abonați la topicurile corespunzătoare.
---

## Structura proiectului

### Fișiere principale

1. **`server.cpp`**:
   - Conține implementarea serverului, inclusiv gestionarea conexiunilor, subscripțiilor și trimiterea mesajelor.

2. **`tcp_client.cpp`**:
   - Implementarea clientului TCP care permite utilizatorilor să interacționeze cu serverul.
   - Clienții pot trimite comenzi precum `subscribe`, `unsubscribe` și `exit`.

3. **`utils.cpp`**:
   - Conține funcții auxiliare utilizate atât de server, cât și de client pentru trimiterea și primirea completă a datelor prin socket-uri.

4. **`server.hpp`**:
   - Declarațiile funcțiilor și structurilor utilizate în server.

5. **`tcp_client.hpp`**:
   - Declarațiile funcțiilor și structurilor utilizate în client.

6. **`utils.hpp`**:
   - Declarațiile funcțiilor auxiliare din `utils.cpp`.

---
## Funcții importante în `server.cpp`

### 1. `setup_server`
Această funcție configurează socket-urile TCP și UDP și le leagă la portul specificat. Este responsabilă pentru inițializarea serverului astfel încât să poată asculta conexiuni și mesaje.

- **Pași principali**:
  1. Creează un socket TCP folosind `socket(AF_INET, SOCK_STREAM, 0)`.
  2. Creează un socket UDP folosind `socket(AF_INET, SOCK_DGRAM, 0)`.
  3. Configurează adresa serverului (`struct sockaddr_in`) pentru a asculta pe toate interfețele (`INADDR_ANY`) și pe portul specificat.
  4. Leagă socket-urile TCP și UDP la adresa configurată folosind `bind`.
  5. Pune socket-ul TCP în modul de ascultare folosind `listen`.

- **Rol**:
  - Pregătește serverul pentru a accepta conexiuni TCP și pentru a primi mesaje UDP.

---

### 2. `handle_tcp`
Această funcție gestionează conexiunile noi de la clienți pe socket-ul TCP. Este apelată atunci când serverul detectează activitate pe socket-ul TCP.

- **Pași principali**:
  1. Acceptă o conexiune nouă folosind `accept`.
  2. Configurează socket-ul clientului pentru a permite reutilizarea adresei (`SO_REUSEADDR`) și pentru a dezactiva algoritmul Nagle (`TCP_NODELAY`).
  3. Adaugă socket-ul clientului în setul de file descriptori activi (`FD_SET`) și actualizează valoarea maximă a file descriptorilor (`fd_max`).

- **Rol**:
  - Permite serverului să accepte conexiuni noi de la clienți și să le gestioneze ulterior.

---

### 3. `handle_udp`
Această funcție primește mesaje pe socket-ul UDP și le trimite către clienții abonați la topicurile corespunzătoare.

- **Pași principali**:
  1. Primește un mesaj UDP folosind `recvfrom`.
  2. Extrage topicul din mesaj și verifică dacă există clienți abonați la topicurile care se potrivesc.
  3. Utilizează un `std::set` pentru a evita trimiterea de mesaje duplicate către același client.
  4. Trimite mesajul către fiecare client abonat folosind `send_all`.

- **Rol**:
  - Asigură livrarea mesajelor primite pe socket-ul UDP către clienții abonați.

---

### 4. `handle_connect`
Această funcție gestionează conectarea unui client nou sau reconectarea unui client existent.

- **Pași principali**:
  1. Verifică dacă clientul este deja înregistrat în lista de clienți (`clients`).
  2. Dacă clientul este nou, creează o structură `client_data_t` pentru a stoca informațiile despre client și îl adaugă în lista de clienți.
  3. Dacă clientul este deja conectat, închide conexiunea nouă și afișează un mesaj de avertizare.
  4. Dacă clientul este deconectat, actualizează starea acestuia și reia conexiunea.

- **Rol**:
  - Permite gestionarea atât a clienților noi, cât și a celor care se reconectează.

---

### 5. `handle_subscribe`
Această funcție adaugă un client în lista de abonați pentru un topic specific.

- **Pași principali**:
  1. Găsește clientul în lista de clienți folosind ID-ul acestuia.
  2. Adaugă clientul în lista de abonați pentru topicul specificat.

- **Rol**:
  - Permite clienților să se aboneze la topicuri pentru a primi mesaje relevante.

---

### 6. `handle_unsubscribe`
Această funcție elimină un client din lista de abonați pentru topicurile care se potrivesc cu pattern-ul specificat.

- **Pași principali**:
  1. Iterează prin toate topicurile din `topics_to_clients`.
  2. Verifică dacă topicul curent se potrivește cu pattern-ul specificat folosind funcția `topic_matches`.
  3. Elimină clientul din lista de abonați pentru topicurile care se potrivesc.

- **Rol**:
  - Permite clienților să se dezaboneze de la topicuri.

---

### 7. `handle_exit`
Această funcție gestionează deconectarea unui client, marcându-l ca deconectat.

- **Pași principali**:
  1. Găsește clientul în lista de clienți folosind ID-ul acestuia.
  2. Marchează clientul ca deconectat și închide conexiunea.
  3. Elimină socket-ul clientului din setul de file descriptori activi (`FD_CLR`).

- **Rol**:
  - Asigură gestionarea corectă a deconectării clienților.

---

### 8. `run_server`
Aceasta este bucla principală a serverului care gestionează evenimentele de pe socket-uri și stdin.

- **Pași principali**:
  1. Inițializează setul de file descriptori activi și așteaptă evenimente folosind `select`.
  2. Verifică dacă există activitate pe socket-ul TCP, socket-ul UDP sau stdin.
  3. Apelează funcțiile corespunzătoare (`handle_tcp`, `handle_udp`, `handle_clients`) pentru a gestiona evenimentele detectate.
  4. Permite oprirea serverului prin comanda `exit` introdusă în stdin.

- **Rol**:
  - Coordonează toate activitățile serverului, inclusiv gestionarea conexiunilor și procesarea mesajelor.

---

## Funcții importante în `tcp_client.cpp`

### 1. `setup_connection`
Această funcție configurează conexiunea TCP cu serverul. Clientul trimite un mesaj de tip `CONNECT` pentru a se înregistra pe server.

- **Pași principali**:
  1. Creează un socket TCP și se conectează la server folosind `connect`.
  2. Trimite un mesaj de tip `CONNECT` către server pentru a se înregistra.

- **Rol**:
  - Permite clientului să se conecteze la server și să se înregistreze.

---

### 2. `handle_server_message`
Această funcție primește și procesează mesajele trimise de server. Mesajele sunt afișate utilizatorului.

- **Pași principali**:
  1. Primește mesajele de la server folosind `recv_all`.
  2. Procesează mesajele primite și le afișează utilizatorului.

- **Rol**:
  - Asigură comunicarea între server și client.

---

### 3. `handle_user_input`
Această funcție gestionează comenzile introduse de utilizator.

- **Comenzi suportate**:
  - `subscribe <topic>`: Abonează clientul la un topic.
  - `unsubscribe <topic>`: Dezabonează clientul de la un topic.
  - `exit`: Închide conexiunea cu serverul.

- **Pași principali**:
  1. Citește comanda introdusă de utilizator.
  2. Trimite comanda către server pentru procesare.

- **Rol**:
  - Permite utilizatorului să interacționeze cu serverul.

---

### 4. `run_tcp_client`
Aceasta este bucla principală a clientului care gestionează interacțiunea cu serverul și utilizatorul.

- **Pași principali**:
  1. Așteaptă evenimente de la server sau de la utilizator.
  2. Procesează mesajele primite de la server.
  3. Procesează comenzile introduse de utilizator.

- **Rol**:
  - Coordonează toate activitățile clientului, inclusiv comunicarea cu serverul și interacțiunea cu utilizatorul.

## Wildcard-uri pentru topicuri

### `+` (Wildcard pentru un segment)
- Se potrivește cu exact un segment al unui topic.
- Exemplu:
  - Subscriere: `upb/+/temperature`
  - Mesaje care se potrivesc:
    - `upb/ec/temperature`
    - `upb/room/temperature`
  - Mesaje care **nu** se potrivesc:
    - `upb/ec/room/temperature`

### `*` (Wildcard pentru zero sau mai multe segmente)
- Se potrivește cu zero sau mai multe segmente.
- Exemplu:
  - Subscriere: `upb/*`
  - Mesaje care se potrivesc:
    - `upb/ec/temperature`
    - `upb/ec/room/temperature`
    - `upb`

---

## Structuri de date

### 1. `clients` (în `server.cpp`)
- Tip: `std::map<std::string, client_data_t *>`
- Descriere: Stochează informațiile despre clienți, indexate după ID-ul clientului.

### 2. `topics_to_clients` (în `server.cpp`)
- Tip: `std::map<std::string, std::vector<client_data_t *>>`
- Descriere: Stochează lista de clienți abonați pentru fiecare topic.

---

## Fluxul principal al serverului

1. **Inițializare**:
   - Serverul configurează socket-urile TCP și UDP și începe să asculte pentru conexiuni și mesaje.

2. **Gestionarea conexiunilor**:
   - Clienții se conectează prin TCP, iar serverul le gestionează subscripțiile.

3. **Primirea mesajelor**:
   - Mesajele primite pe socket-ul UDP sunt procesate și trimise către clienții abonați.

4. **Deconectarea clienților**:
   - Clienții se pot deconecta, iar serverul păstrează subscripțiile lor pentru o eventuală reconectare.

5. **Închiderea serverului**:
   - Serverul poate fi oprit prin comanda `exit` introdusă în stdin.

---

## Fluxul principal al clientului

1. **Inițializare**:
   - Clientul configurează conexiunea TCP cu serverul.

2. **Interacțiunea cu utilizatorul**:
   - Clientul așteaptă comenzi de la utilizator (`subscribe`, `unsubscribe`, `exit`).

3. **Primirea mesajelor**:
   - Clientul primește mesaje de la server și le afișează utilizatorului.

4. **Închiderea conexiunii**:
   - Clientul trimite un mesaj de tip `EXIT` către server și închide conexiunea.

