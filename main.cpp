#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <random>

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 32; // Size of each tile (this creates a retro feel)
const int MAX_TILES_X = SCREEN_WIDTH / TILE_SIZE;
const int MAX_TILES_Y = SCREEN_HEIGHT / TILE_SIZE;
const int FPS = 60;

// Hall of Fame player data (Real players with fake scores)
struct Position {
    int x, y;
};

struct Player {
    std::string name;
    int score;
};

// Fun facts about the Redskins
std::vector<std::string> funFacts = {
    "The Washington Redskins were founded in 1932.",
    "Joe Theismann won the NFL MVP in 1983.",
    "The Redskins have 5 Super Bowl appearances.",
    "Art Monk is a Hall of Fame wide receiver for the Redskins.",
    "The Washington Football team changed its name in 2020."
};

// Function to load images
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(path.c_str());
    if (!tempSurface) {
        std::cout << "Error loading image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}

// Helper to render textures
void renderTexture(SDL_Texture* tex, SDL_Renderer* renderer, int x, int y) {
    SDL_Rect dest = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
}

// Function to render the grid (retro effect)
void renderGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);  // Dark gray grid lines
    for (int x = 0; x < MAX_TILES_X; ++x) {
        SDL_RenderDrawLine(renderer, x * TILE_SIZE, 0, x * TILE_SIZE, SCREEN_HEIGHT);  // Vertical grid lines
    }
    for (int y = 0; y < MAX_TILES_Y; ++y) {
        SDL_RenderDrawLine(renderer, 0, y * TILE_SIZE, SCREEN_WIDTH, y * TILE_SIZE);  // Horizontal grid lines
    }
}

// Function to render text
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color textColor = {255, 255, 255};  // White text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    if (!textSurface) {
        std::cout << "Text rendering error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_Rect destRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &destRect);
    SDL_DestroyTexture(textTexture);
}

// Function to display the game over screen
void gameOver(SDL_Renderer* renderer, TTF_Font* font, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    renderText(renderer, font, "Game Over! Final Score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

    SDL_RenderPresent(renderer);
    SDL_Delay(3000); // Show for 3 seconds
}

// Function to save the score
void saveScore(int score) {
    std::ofstream scoreFile("scores.txt", std::ios::app);
    if (scoreFile.is_open()) {
        scoreFile << score << "\n";
        scoreFile.close();
    }
}

// Function to load Hall of Fame (fake scores)
std::vector<Player> loadHallOfFame() {
    std::vector<Player> hallOfFame = {
        {"Joe Theismann", 100},
        {"Darrell Green", 95},
        {"Art Monk", 90},
        {"John Riggins", 85},
        {"Champ Bailey", 80}
    };

    // Sort players by score in descending order
    std::sort(hallOfFame.begin(), hallOfFame.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
    });

    return hallOfFame;
}

// Function to display the Hall of Fame
void displayHallOfFame(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    std::vector<Player> hallOfFame = loadHallOfFame();

    renderText(renderer, font, "Hall of Fame", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4);

    int yOffset = SCREEN_HEIGHT / 3;
    for (const auto& player : hallOfFame) {
        renderText(renderer, font, player.name + " - " + std::to_string(player.score), SCREEN_WIDTH / 4, yOffset);
        yOffset += 30; // Adjust spacing
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(5000); // Show Hall of Fame for 5 seconds
}

// Function to get a random fun fact
std::string getRandomFunFact() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, funFacts.size() - 1);
    return funFacts[dis(gen)];
}

// Function to check if the food spawns on the snake
bool isFoodOnSnake(const Position& food, const std::vector<Position>& snakeBody) {
    for (const auto& segment : snakeBody) {
        if (food.x == segment.x && food.y == segment.y) {
            return true; // Food is on the snake
        }
    }
    return false;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("3D-Like Snake Game - Redskins Helmet",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Load images
    SDL_Texture* helmetTexture = loadTexture("redskins_helmet.png", renderer);
    SDL_Texture* footballTexture = loadTexture("football.png", renderer);

    if (!helmetTexture || !footballTexture) {
        std::cout << "Error loading images" << std::endl;
        return 1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Make sure you have a font file in the correct location
    if (!font) {
        std::cout << "Error loading font: " << TTF_GetError() << std::endl;
        return 1;
    }

    // Game state variables
    std::vector<Position> snakeBody;  // Snake body
    Position food = {MAX_TILES_X / 2, MAX_TILES_Y / 2};  // Football position
    Position head = {MAX_TILES_X / 4, MAX_TILES_Y / 2};  // Start position of helmet

    snakeBody.push_back(head);  // The first helmet is at the starting position
    bool quit = false;
    SDL_Event event;
    int score = 0;
    int dx = 1, dy = 0;  // Direction of movement (right)

    // Timing variables to control the snake's speed
    auto lastMoveTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds moveInterval(300); // Delay between each snake movement

    // Timer to track elapsed time
    auto startTime = std::chrono::steady_clock::now();

    // Timer for fun facts display (change every 10 seconds)
    auto lastFactTime = std::chrono::steady_clock::now();
    const std::chrono::seconds factInterval(10); // Interval for changing the fun fact

    std::string currentFunFact = getRandomFunFact(); // Initial fun fact

    // Game loop
    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(renderer);

        renderGrid(renderer); // Render grid background

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LEFT && dx == 0) {
                    dx = -1; dy = 0;
                } else if (event.key.keysym.sym == SDLK_RIGHT && dx == 0) {
                    dx = 1; dy = 0;
                } else if (event.key.keysym.sym == SDLK_UP && dy == 0) {
                    dx = 0; dy = -1;
                } else if (event.key.keysym.sym == SDLK_DOWN && dy == 0) {
                    dx = 0; dy = 1;
                }
            }
        }

        // Check if it's time to move the snake
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - lastMoveTime >= moveInterval) {
            // Move the head of the helmet
            head.x += dx;
            head.y += dy;

            // Check for collision with the walls
            if (head.x < 0 || head.x >= MAX_TILES_X || head.y < 0 || head.y >= MAX_TILES_Y) {
                saveScore(score);  // Save score to file
                gameOver(renderer, font, score);
                displayHallOfFame(renderer, font);  // Display the Hall of Fame after game over
                quit = true;
                break;
            }

            // Check if the helmet eats the football (food)
            if (head.x == food.x && head.y == food.y) {
                score++;  // Increase score
                snakeBody.push_back(Position{food.x, food.y});  // Add new helmet to the body
                do {
                    food = {rand() % MAX_TILES_X, rand() % MAX_TILES_Y};  // Spawn new food
                } while (isFoodOnSnake(food, snakeBody));  // Ensure food is not on the snake
            }

            // Move the rest of the helmets
            for (size_t i = snakeBody.size() - 1; i > 0; --i) {
                snakeBody[i] = snakeBody[i - 1];
            }
            snakeBody[0] = head;

            // Update last move time
            lastMoveTime = currentTime;
        }

        // Change fun fact every 10 seconds
        if (currentTime - lastFactTime >= factInterval) {
            currentFunFact = getRandomFunFact();
            lastFactTime = currentTime;
        }

        // Render all helmets
        for (const auto& helmet : snakeBody) {
            renderTexture(helmetTexture, renderer, helmet.x, helmet.y);
        }

        // Render the food (football)
        renderTexture(footballTexture, renderer, food.x, food.y);

        // Render score and time elapsed
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        renderText(renderer, font, "Score: " + std::to_string(score), 10, 10);
        renderText(renderer, font, "Time: " + std::to_string(elapsed) + "s", 10, 40);

        // Render the current fun fact
        renderText(renderer, font, currentFunFact, SCREEN_WIDTH / 4, 10);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);  // Limit the frame rate
    }

    // Clean up
    SDL_DestroyTexture(helmetTexture);
    SDL_DestroyTexture(footballTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
