// Include important C++ libraries here
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <optional>
#include <complex>      // std::complex, std::abs
#include <thread>
#include "ComplexPlane.h"
#include <random>
#include <atomic>
#include <mutex>
#include <condition_variable>
using namespace sf;
using namespace std;

//Window size constants
// constexpr unsigned WINDOW_WIDTH = 1920u;
// constexpr unsigned WINDOW_HEIGHT = 1080u;
// float FWINDOW_WIDTH = static_cast<float>(WINDOW_WIDTH);
// float FWINDOW_HEIGHT = static_cast<float>(WINDOW_HEIGHT);
const string FONT_PATH = "/Users/jacksmachine/Documents/School/cmake-sfml-project/src/ArialHB.ttc";
[[noreturn]] void recalculateThread(ComplexPlane &_plane) {
	while (true) {
		unique_lock<mutex> lock(_plane.mutex);
		_plane.cv.wait(lock, [&_plane] { return _plane.workToDo; });
		_plane.workToDo = false;
		lock.unlock();
		//cout << "[Worker] Recalculating plane at zoom: " << currentZoom << "\n";
		_plane.updateRender();
		//cout << "[Worker] Render done." << "\n";
	}
}



int main() {
	VideoMode desktop = VideoMode::getDesktopMode();
	unsigned int WINDOW_WIDTH = desktop.size.x;
	unsigned int WINDOW_HEIGHT = desktop.size.y;
	// Create a video mode object
	auto window = RenderWindow(desktop, "MandelBrot");
	//auto window = sf::RenderWindow(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "MandelBrot");

	// Construct ComplexPlane with pixel width then height to match screen dimensions
	ComplexPlane plane(static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT));
	RectangleShape screenQuad;
	screenQuad.setSize(Vector2f(window.getSize()));

	screenQuad.setPosition({0.f,0.f});

	// Load the text font https://www.sfml-dev.org/tutorials/3.0/graphics/text/
	Font MyFont;
	if (!MyFont.openFromFile("/Users/jacksmachine/Documents/School/cmake-sfml-project/src/NewYork.ttf")) {
		std::cerr << "Error could not load font";
		return 0;
	}
	Text text(MyFont);


	// worker on flag
	atomic<bool> isRunning(true);
	//create worker thread pass complexPlane object by reference
	thread worker(recalculateThread, ref(plane));

	// lambda functions passed to sfml sf::WindowBase::handleEvents https://www.sfml-dev.org/tutorials/3.0/window/events/
	const auto onClose = [&window, &isRunning](const sf::Event::Closed &) {
		window.close();
		isRunning = false;
	};
	const auto onMousePressed = [&plane](const sf::Event::MouseButtonPressed &mousePressed) {
		plane.workToDo = true;
		if (mousePressed.button == sf::Mouse::Button::Left) {
			const auto mouseX = static_cast<float>(mousePressed.position.x);
			const auto mouseY = static_cast<float>(mousePressed.position.y);
			std::cout << "Left mouse button pressed" << std::endl;
			std::cout << "mouse x: " << mouseX << std::endl;
			std::cout << "mouse y: " << mouseY << std::endl;
			lock_guard<std::mutex> lock(plane.mutex);
			plane.setCenter(mousePressed.position);
			plane.zoomIn();
		}
		if (mousePressed.button == sf::Mouse::Button::Right) {
			std::cout << "right mouse button pressed" << std::endl;
			std::cout << "mouse x: " << mousePressed.position.x << "\n";
			std::cout << "mouse y: " << mousePressed.position.y << "\n";
			plane.setCenter(mousePressed.position);
			plane.zoomOut();
		}
	};
	const auto onMouseMoved = [&plane](const sf::Event::MouseMoved &mouse_moved) {
		plane.setMouseLocation(mouse_moved.position);
	};
	const auto MouseWheelMoved = [&plane](const sf::Event::MouseWheelScrolled &mouse_wheel_scrolled) {

		if (mouse_wheel_scrolled.delta < 0) {
			cout << "mouse wheel scrolled down";
			plane.zoomOut();
		} else if (mouse_wheel_scrolled.delta > 0) {
			cout << "mouse wheel scrolled up";
			plane.zoomIn();
		}
	};
	const auto onKeyPressed = [&plane](const sf::Event::KeyPressed &key_pressed)
	{
		cout << "keyPressed\n";

		const Vector3f v = {0.1, 0.1, 0.1};
		bool shiftHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
						 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
		if (shiftHeld) {
			if (key_pressed.code == sf::Keyboard::Key::Left) {

				plane.offset -= v;
			}
			if (key_pressed.code == sf::Keyboard::Key::Up) {

				plane.amp -= v;
			}
			if (key_pressed.code == sf::Keyboard::Key::Down) {

				plane.freq -= v;
			}
			if (key_pressed.code == sf::Keyboard::Key::Right) {

				plane.phase -= v;
			}
		}
		if (key_pressed.code == sf::Keyboard::Key::Left) {

			plane.offset += v;
		}
		if (key_pressed.code == sf::Keyboard::Key::Up) {

			plane.amp += v;
		}
		if (key_pressed.code == sf::Keyboard::Key::Down) {

			plane.freq += v;
		}
		if (key_pressed.code == sf::Keyboard::Key::Right) {

			plane.phase += v;
		}
		if (key_pressed.code == sf::Keyboard::Key::R) {
			plane.offset = {0.5, 0.5, 0.5};
			plane.amp = {0.5, 0.5, 0.5};
			plane.freq = {1.0, 1.0, 1.0};
			plane.phase = {0.80, 0.90, 0.30};
		}
		if (key_pressed.code == sf::Keyboard::Key::A) {
			plane.offset = {0.5f, 0.5f, 0.5f};
			plane.amp    = {0.5f, 0.5f, 0.5f};


			plane.freq   = {1.0f, 1.0f, 0.5f};


			plane.phase  = {0.80f, 0.90f, 0.30f};
		}
		plane.workToDo = true;
		plane.m_State = CALCULATING;
		plane.cv.notify_one();


	};
	// Start the window loop
	while (window.isOpen()) {
		/*
		****************************************
		Handle the players input
		****************************************
		*/
		window.handleEvents(onClose, onMousePressed, onMouseMoved, onKeyPressed);
		/*
		****************************************
		Draw
		****************************************
		*/
		plane.loadText(text);
		window.clear();
		//window.draw(plane);
		window.draw(screenQuad, &plane.mandelbrotShader);
		window.draw(text);
		window.display();
	}
	// join worker with main thread to prevent crash at program close.
	if (worker.joinable()) {
		worker.join();
	}
}

