#include "Chip8.h"
#include <SFML/Graphics.hpp>

// The object by which we refer to the entire system
Chip8 chip8;

// Screen dimension constants
const int SCREEN_WIDTH = 10 * 64;
const int SCREEN_HEIGHT = 10 * 32;
const int SQUARE_SIDE = 10;

// The window we will be rendering the output to
sf::RenderWindow window;

// Fixes, clears and then makes the window opaque
void setupGraphics()
{ 
    window.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Shit-8");
    window.clear();
    window.display();
}

// Uses state of machine to render data
void drawGraphics()
{ 
    window.clear();
    std::vector<unsigned char> v = chip8.getGFXArray(); // Function which returns state of VRAM

    // Iterating through the single dimension array as though it were two-dimensional
    for (int y = 0; y < 32; ++y) {
	for (int x = 0; x < 64; ++x) {
	    if (v[y * 64 + x] == 1) {						  // Here we make use of our way of interpreting the array and simply interpret every 64th line as a newline
		sf::RectangleShape shape(sf::Vector2f(SQUARE_SIDE, SQUARE_SIDE)); // Create the rectangle to be rendered
		shape.setFillColor(sf::Color::White);				  // Set the fill color
		shape.setPosition(x * SQUARE_SIDE, y * SQUARE_SIDE);		  // Assign the position
		window.draw(shape);
	    }
	}
    }
    window.display(); // Now we display our result
}

int main(int argc, char** argv)
{
    setupGraphics(); // Ready up our window

	string rom = argv[1];
    chip8.loadROM(rom); // Load the ROM from the given path

	// SFML specific, when window is closed condition is returned (after the event is handled) false
    while (window.isOpen()) 
	{ 
		sf::Clock clock; // SFML clock object to use as a timer
		sf::Time currTime = clock.getElapsedTime();
		sf::Event event; // SFML event object to listen for a clsoing event

		while (chip8.getChipState()) 
		{
			currTime = clock.getElapsedTime();

			if (currTime.asSeconds() >= 0.002) // Make sure an adequate amount of time has passed since last cycle
			{
					currTime = clock.restart(); // Reset the timer
					chip8.printDebug();
					chip8.emulateCycle();       // Go through with a single emulation cycle
					cout << "Emulated" << endl;

					if (chip8.getDrawFlag()) 
					{    // If set a change has been made in VRAM and needs an update in visual representation
						drawGraphics();	   // Get VRAM and draw visual representation
						chip8.setDrawFlag(false); // With the update we need not draw anymore as of right now
					}
			}

			// Check all the window's events that were triggered since the last iteration of the loop
			while (window.pollEvent(event)) 
			{
					// "Close requested" event: we close the window and kill emulation
					if (event.type == sf::Event::Closed) 
					{
						window.close();
						chip8.shutdown(); // Sets flag of machine to return false when calling getChipState()
					}
			}
		}
    }
}
