#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime> // For random seed

// Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 32;
const int MAX_TILES_X = SCREEN_WIDTH / TILE_SIZE;
const int MAX_TILES_Y = SCREEN_HEIGHT / TILE_SIZE;
const int FPS = 60;

// Hall of Fame player data
struct Player {
    std::string name;
    int score;
};

// Position struct to hold x and y coordinates
struct Position {
    int x, y;
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

// Placeholder renderText function (You can improve it using SDL_ttf later)
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y) {
    // For now, just print text to the console
    std::cout << text << std::endl;
}

// Function to display the game over screen
void gameOver(SDL_Renderer* renderer, int score) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    renderText(renderer, "Game Over! Final Score: " + std::to_string(score), SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);
    
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

    std::sort(hallOfFame.begin(), hallOfFame.end(), [](const Player& a, const Player& b) {
        return a.score > b.score;
    });

    return hallOfFame;
}

// Function to display the Hall of Fame
void displayHallOfFame(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    std::vector<Player> hallOfFame = loadHallOfFame();

    renderText(renderer, "Hall of Fame", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 4);
    
    int yOffset = SCREEN_HEIGHT / 3;
    for (const auto& player : hallOfFame) {
        renderText(renderer, player.name + " - " + std::to_string(player.score), SCREEN_WIDTH / 4, yOffset);
        yOffset += 30;
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(5000); // Show Hall of Fame for 5 seconds
}

// Function to generate random food position
Position getRandomFoodPosition(const std::vector<Position>& helmets) {
    Position food;
    bool collision;

    do {
        collision = false;
        food.x = rand() % MAX_TILES_X;
        food.y = rand() % MAX_TILES_Y;

        // Check if the food overlaps with any part of the helmet (snake)
        for (const auto& helmet : helmets) {
            if (food.x == helmet.x && food.y == helmet.y) {
                collision = true;
                break;
            }
        }
    } while (collision);

    return food;
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Initialize random seed

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("3D-Like Snake Game - Redskins Helmet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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

    // Game state variables
    std::vector<Position> helmets;
    Position food = {MAX_TILES_X / 2, MAX_TILES_Y / 2}; 
    Position head = {MAX_TILES_X / 4, MAX_TILES_Y / 2};

    helmets.push_back(head);
    bool quit = false;
    SDL_Event event;
    int score = 0;
    int dx = 1, dy = 0;

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);

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

        head.x += dx;
        head.y += dy;

        if (head.x < 0 || head.x >= MAX_TILES_X || head.y < 0 || head.y >= MAX_TILES_Y) {
            saveScore(score);
            gameOver(renderer, score);
            displayHallOfFame(renderer);
            quit = true;
            break;
        }

        if (head.x == food.x && head.y == food.y) {
            score++;
            helmets.push_back(Position{food.x, food.y});
            food = getRandomFoodPosition(helmets);  // Get new food position
        }

        for (size_t i = helmets.size() - 1; i > 0; --i) {
            helmets[i] = helmets[i - 1];
        }
        helmets[0] = head;

        for (const auto& helmet : helmets) {
            renderTexture(helmetTexture, renderer, helmet.x, helmet.y);
        }

        renderTexture(footballTexture, renderer, food.x, food.y);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyTexture(helmetTexture);
    SDL_DestroyTexture(footballTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
