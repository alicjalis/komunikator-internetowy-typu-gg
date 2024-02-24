# Komunikator Internetowy Typu GG
## Opis
Komunikator Internetowy Typu GG to 
projekt zrobiony na przedmiot sieci 
komputerowe 2. Jest to prosta wersja chatu,
w architekturze klient-serwer, z 
wykorzystaniem protolołu TCP. Serwer jest współbieżny. Połączenie
klienta z serwerem jest realizowane za pomocą
gniazd serwerowych sockets.

## Szczegóły techniczne

- Serwer: C, BSD Sockets, Linux
- Klient: Python, Winsock, Windows

## Kompilacja
W systemie operacyjnnym Linux należy pobrać oraz odpalić
plik `server.c`. Przed kompilacją i
uruchomieniem należy ustawić w kodzie odpowiedni
numer portu.

Do kompilacji serwera należy użyć polecenia:

`gcc -pthread -o server server.c`

Aby uruchomić serwer należy użyć polecenia:

`./server`

W systemie operacyjnym Windows należy odpalić
plik `main.py` i zmienić adres ip w
kodzie na adres drugiego
komputera. 

W celu uruchomienia programu klienta
należy wpisać:

`python main.py`

## Działanie programu
Po odpaleniu program poprosi o wpisanie
nickname. Wyswietlone zostana nieodebrane wiadomosci, a program prosi o nickname osoby do ktorej chcemy napisac 
wiadomosc. Można wpisać nickname, lub kliknąć na nieodebraną wiadomość w celu przejścia do konwersacji. 




