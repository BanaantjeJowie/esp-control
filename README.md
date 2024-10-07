# Room control using ESP32

In this project I made the lighting in my room more advanced and controllable by connecting them to wifi using ESP32's and some relays.
A central "server" is connected to all the relays of my main light, LED-strips and possibly more ambient lights or RGB strips in the future. This server hosts a webpage on the local network to control the lights. The server can also communicate with other ESP's using http requests. Every light has its own request soit can be toggled. In my instance I used another ESP as a client that sends requests to toggle the main light, this can be changed by using the right http request for the light you want.

The Gihub page is a preview of the website hosted on the network.