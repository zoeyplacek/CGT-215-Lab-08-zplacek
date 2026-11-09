#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>

using namespace std;
using namespace sf;
using namespace sfp;

const float DUCK_SPEED = 0.40;   // pixels per millisecond
const int DUCK_DELAY_MS = 750;  // time between duck spawning

void LoadTex(Texture& tex, string filename) { // loads a texture and prints an error if it fails
	if (!tex.loadFromFile(filename)) {
		cout << "Could not load " << filename << endl;
	}
}

Vector2f GetTextSize(Text text) { // returns the size of a text object
	FloatRect r = text.getGlobalBounds();
	return Vector2f(r.width, r.height);
}

int main()
{
	// creating window
	RenderWindow window(VideoMode(800, 600), "Duck Hunter"); // window size is now 800x600
	World world(Vector2f(0, 0));   // no gravity
	int score(0);                  // score starts at 0
	int arrows(5);                 // 5 shots total
	bool gameOver(false);          // game over when arrows = 0 
	bool spaceReleased(false);     // detects when space bar is released after game over to allow user to exit screen

	// arial font
	Font fnt;
	if (!fnt.loadFromFile("arial.ttf")) { // loading font from file
		cout << "Could not load font." << endl;
		exit(3);
	}

	// crossbow creation
	PhysicsSprite crossBow;
	Texture cbowTex; // loading crossbow texture from file
	LoadTex(cbowTex, "images/crossbow.png");
	crossBow.setTexture(cbowTex);
	Vector2f sz = crossBow.getSize(); // getting size of crossbow texture
	crossBow.setCenter(Vector2f(400, 600 - (sz.y / 2))); // crossbow position at bottom center of window

	// arrow creation
	PhysicsSprite arrow;
	Texture arrowTex;
	LoadTex(arrowTex, "images/arrow.png");
	arrow.setTexture(arrowTex);
	bool drawingArrow(false); // true when an arrow is in flight

	// top wall
	PhysicsRectangle top;
	top.setSize(Vector2f(800, 10)); // constraint
	top.setCenter(Vector2f(400, 5)); // constraint
	top.setStatic(true); // top wall does not move
	world.AddPhysicsBody(top); // adding top wall to world

	top.onCollision = [&drawingArrow, &world, &arrow] // lambda function to remove arrow when it collides with top wall
	(PhysicsBodyCollisionResult result) { // when arrow hits top wall
		drawingArrow = false; // arrow is no longer in flight
		world.RemovePhysicsBody(arrow); // removing arrow from world
		};

	// right wall
	PhysicsRectangle right;
	right.setSize(Vector2f(10, 600)); // contraint
	right.setCenter(Vector2f(900, 300)); // contraint
	right.setStatic(true); // right wall does not move
	world.AddPhysicsBody(right); // adding right wall to world

	// duck creation
	Texture duckTex;
	LoadTex(duckTex, "images/duck.png");
	PhysicsShapeList<PhysicsSprite> ducks;
	long duckMS(0);  // time since last duck spawned

	// game timer
	Clock clock; // starts the clock
	Time lastTime(clock.getElapsedTime()); // initial time
	Time currentTime(lastTime); // current time

	while (window.isOpen()) {
		currentTime = clock.getElapsedTime();
		Time deltaTime = currentTime - lastTime;
		long deltaMS = deltaTime.asMilliseconds();

		if (deltaMS > 9) { // only update the game every 10 milliseconds
			lastTime = currentTime;

			if (!gameOver) { // game is frozen when gameOver is true
				world.UpdatePhysics(deltaMS); // update the physics world
				duckMS = duckMS + deltaMS; // increment the time since last duck spawned
				if (duckMS > DUCK_DELAY_MS) { // time to spawn a new duck
					duckMS = 0;   // reset counter for next duck
					PhysicsSprite& duck = ducks.Create(); // create a new duck
					duck.setTexture(duckTex);
					Vector2f dsz = duck.getSize();
					duck.setCenter(Vector2f(-(dsz.x / 2), 20 + (dsz.y / 2))); // start the duck to the left side of the window
					duck.setVelocity(Vector2f(DUCK_SPEED, 0)); // moves the duck to the right at a constant speed
					world.AddPhysicsBody(duck); // add duck to world
					// collision for duck
					duck.onCollision =
						[&drawingArrow, &world, &arrow, &right,
						&duck, &ducks, &score]
						(PhysicsBodyCollisionResult result) {
						if (result.object2 == arrow) { // duck was hit by the arrow
							drawingArrow = false; // arrow is no longer in flight
							world.RemovePhysicsBody(arrow); // removes arrow
							world.RemovePhysicsBody(duck); // removes duck
							ducks.QueueRemove(duck);
							score += 10; // score is updated by 10
						}

						if (result.object2 == right) { // duck reached the right wall
							world.RemovePhysicsBody(duck); // removes duck
							ducks.QueueRemove(duck);
						}
						};
				}

				// shooting arrow
				if (Keyboard::isKeyPressed(Keyboard::Space) && // when space bar is pressed
					!drawingArrow && arrows > 0) {  // only shoot if an arrow is not already in flight and there are arrows left
					drawingArrow = true; // arrow is now in flight
					arrows = arrows - 1;                     // one arrow used
					arrow.setCenter(crossBow.getCenter());   // start at the crossbow
					arrow.setVelocity(Vector2f(0, -1));      // fly straight up
					world.AddPhysicsBody(arrow); // add arrow to world
				}

				ducks.DoRemovals(); // removes any ducks that were hit or reached the right wall

				if (!((arrows > 0) || drawingArrow)) { // if there are no arrows left and the arrow is not in flight, the game is over
					gameOver = true; // game is now frozen
				}
			}

			// drawing the game
			window.clear();
			if (drawingArrow) {
				window.draw(arrow);     // only draw the arrow while it's flying
			}

			for (PhysicsShape& duck : ducks) { // draw all the ducks
				window.draw((PhysicsSprite&)duck);
			}
			window.draw(crossBow);

			// arrow count
			Text arrowCountText;
			arrowCountText.setString(to_string(arrows)); // display the number of arrows left
			arrowCountText.setFont(fnt); // arial font
			arrowCountText.setPosition(Vector2f(10, 555)); // bottom left corner
			window.draw(arrowCountText); // draw the arrow count text

			// score
			Text scoreText;
			scoreText.setString(to_string(score)); // display the score
			scoreText.setFont(fnt); // arial font
			scoreText.setPosition(Vector2f(790 - GetTextSize(scoreText).x, 555)); // bottom right corner
			window.draw(scoreText); // draw the score text

			// game over
			if (gameOver) {
				Text gameOverText;
				gameOverText.setString("GAME OVER"); // display "GAME OVER"
				gameOverText.setFont(fnt); // in arial font
				gameOverText.setCharacterSize(60); // larger font size
				Vector2f gsz = GetTextSize(gameOverText);
				gameOverText.setPosition(400 - (gsz.x / 2), 300 - (gsz.y / 2)); // in the center of the window
				window.draw(gameOverText); // draw game over 

				if (!Keyboard::isKeyPressed(Keyboard::Space)) { // wait for the user to release the space bar before allowing them to close the window
					spaceReleased = true;
				}
				else if (spaceReleased) {
					window.close();
				}
			}

			// display the drawn window
			window.display();
		}
	}
	return 0;
}