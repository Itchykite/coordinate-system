#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#include <cmath>

// Constants
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

int camX = 0; // Camera X position
int camY = 0; // Camera Y position

float scale = 50.0f; // Scale for zooming

// Function prototypes
bool init(); // Initialize SDL
void close(); // Close SDL

float toScreenX(float x); // Convert world X to screen X
float toScreenY(float y); // Convert world Y to screen Y
void drawAxes(); // Draw axes
void drawGrid(); // Draw grid
void drawFunction(float (*f)(float), float xMin, float xMax, float step, SDL_Color color); // Draw function

// Global variables
bool quit = false; // Flag to quit the program
SDL_Window* window = nullptr; // Window
SDL_Renderer* renderer = nullptr; // Renderer

bool dragging = false; // Flag for dragging
int lastMouseX = 0, lastMouseY = 0; // Last mouse position

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        return false;
    }

    window = SDL_CreateWindow("2D Grid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        SDL_DestroyWindow(window);
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        return false;
    }
   
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT); // Set logical size for rendering

    return true;
}

void close()
{
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_Quit();
}

float toScreenX(float x) 
{
    return static_cast<float>((x - camX) * scale + SCREEN_WIDTH / 2); // Convert world X to screen X
}

float toScreenY(float y) 
{
    return static_cast<float>((SCREEN_HEIGHT / 2) - (y - camY) * scale); // Convert world Y to screen Y
}

void drawAxes()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to white
    SDL_RenderDrawLine(renderer, toScreenX(-200), toScreenY(0), toScreenX(200), toScreenY(0)); // Draw X-axis
    SDL_RenderDrawLine(renderer, toScreenX(0), toScreenY(-200), toScreenX(0), toScreenY(200)); // Draw Y-axis
}

void drawGrid()
{
    float xMin = std::ceil(camX - (SCREEN_WIDTH * 2) / (2 * scale)); // Calculate minimum X
    float xMax = std::ceil(camX + (SCREEN_WIDTH * 2)/ (2 * scale)); // Calculate maximum X
    float yMin = std::ceil(camY - (SCREEN_HEIGHT * 2)/ (2 * scale)); // Calculate minimum Y
    float yMax = std::ceil(camY + (SCREEN_HEIGHT * 2) / (2 * scale)); // Calculate maximum Y

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Set color to gray
    for (float x = xMin; x <= xMax; x += 1.0f) 
    {
        SDL_RenderDrawLine(renderer, toScreenX(x), toScreenY(yMin), toScreenX(x), toScreenY(yMax)); // Draw vertical lines
    }

    for (float y = yMin; y <= yMax; y += 1.0f)
    {
        SDL_RenderDrawLine(renderer, toScreenX(xMin), toScreenY(y), toScreenX(xMax), toScreenY(y)); // Draw horizontal lines
    }
}

void drawFunction(float (*f)(float), float xMin, float xMax, float step, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255); // Set color for function
    for (float x = xMin; x < xMax; x += step)
    {
        SDL_RenderDrawLine(renderer, toScreenX(x), toScreenY(f(x)), toScreenX(x + step), toScreenY(f(x + step))); // Draw function lines
    }
}

int main()
{
    if (!init())
    {
        SDL_Log("Failed to initialize! Error: %s\n", SDL_GetError());
    }

    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT) // Handle quit event
            {
                quit = true; 
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) // Exit on escape
            {
                quit = true;
            }
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) // Handle window resize
            {
                SCREEN_WIDTH = e.window.data1;
                SCREEN_HEIGHT = e.window.data2;
                SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
           
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT) // Start draggin on left button down
                {
                    dragging = true; // Start dragging
                    lastMouseX = e.button.x; // Last mouse X position
                    lastMouseY = e.button.y; // Last mouse Y position
                }
            }
            
            else if (e.type == SDL_MOUSEBUTTONUP)
            {
                if (e.button.button == SDL_BUTTON_LEFT) // Stop dragging on left button up
                {
                    dragging = false; // Stop dragging
                }
            }
           
            else if (e.type == SDL_MOUSEMOTION) 
            {
                if (dragging) 
                {
                    int dx = e.motion.x - lastMouseX; // Calculate delta X
                    int dy = e.motion.y - lastMouseY; // Calculate delta Y

                    float panSpeed = 1.5f / std::log2(scale + 2.0f); // Calculate pan speed based on scale
                    camX -= dx * panSpeed; // Update camera X position
                    camY += dy * panSpeed; // Update camera Y position

                    lastMouseX = e.motion.x; // Update last mouse X position
                    lastMouseY = e.motion.y; // Update last mouse Y position
                }
            }

            else if (e.type == SDL_MOUSEWHEEL)
            {
                if (e.wheel.y > 0) // Zoom in on scroll up
                {
                    scale /= 1.1f; // Increase scale
                }
                else if (e.wheel.y < 0) // Zoom out on scroll down
                {
                    scale *= 1.1f; // Decrease scale
                }
            }
        }

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        drawGrid();
        drawAxes();
        drawFunction([](float x) { return x * x; }, -200, 200, 0.1f, {255, 255, 0, 255}); // Draw a sample function (x^2)
        drawFunction([](float x) { return std::sin(x); }, -200, 200, 0.1f, {255, 0, 255, 255}); // Draw a sample function (sin(x))
        drawFunction([](float x) { return std::cos(x); }, -200, 200, 0.1f, {0, 255, 255, 255}); // Draw a sample function (cos(x))
        drawFunction([](float x) { return std::tan(x); }, -200, 200, 0.1f, {0, 255, 0, 255}); // Draw a sample function (tan(x))
        drawFunction([](float x) { return std::exp(x / 100); }, -200, 200, 0.1f, {255, 0, 0, 255}); // Draw a sample function (exp(x))
        drawFunction([](float x) { return std::log(x + 200); }, -200, 200, 0.1f, {0, 0, 255, 255}); // Draw a sample function (log(x))
        drawFunction([](float x) { return std::sqrt(x + 200); }, -200, 200, 0.1f, {255, 255, 255, 255}); // Draw a sample function (sqrt(x))
        drawFunction([](float x) { return std::abs(x); }, -200, 200, 0.1f, {255, 128, 0, 255}); // Draw a sample function (abs(x))

        SDL_RenderPresent(renderer);
    }
}
