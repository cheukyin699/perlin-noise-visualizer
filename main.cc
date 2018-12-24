/* This is the C++ (attempted) implementation of Perlin Noise.
 * I followed the PDF on http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
 *
 * Using SFML, it creates a 600x600 window and fills it with noise values, purple indicating
 * a bigger value, and black indicating a smaller one.
 */
#include <iostream>
#include <random>
#include <array>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace std;

const int WIDTH = 600;
const int HEIGHT = 600;
const int SW = 7;
const int SH = 7;

typedef sf::Vector2<double> Vector2d;

/* Create a random 7x7 grid of gradients, range (-1, 1)
 */
void makeGradients(std::array<Vector2d, SW * SH>& g, double seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> distrib(-1, 1);

    for (unsigned i = 0; i < g.size(); ++i)
    {
        g[i] = Vector2d(distrib(rng), distrib(rng));
    }
}

/* Translate 2D coordinates to 1D array coordinates
 */
int toD(int x, int y)
{
    return y * WIDTH + x;
}

/* Linear interpolation function as specified in the PDF.
 */
double mix(double t)
{
    return 6 * pow(t, 5) - 15 * pow(t, 4) + 10 * pow(t, 3);
}

/* Dot product for vector2d
 */
double dotProduct(Vector2d a, Vector2d b)
{
    return a.x * b.x + a.y * b.y;
}

double clamp(double v)
{
    if (v < -1)
        return -1;
    if (v > 1)
        return 1;
    return v;
}

/* Generate the Perlin Noise
 */
void makePixels(sf::Uint8* ps, std::array<Vector2d, SW * SH>& g)
{
    sf::Uint8* tmp = ps;
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
            // Scale to fit the 6x6 gradients
            double aX = x / 100.0;
            double aY = y / 100.0;
            unsigned i = unsigned(floor(aX));
            unsigned j = unsigned(floor(aY));
            double u = aX - i;
            double v = aY - j;

            auto g00 = g[j * SW + i];
            auto g10 = g[j * SW + i + 1];
            auto g01 = g[(j + 1) * SW + i];
            auto g11 = g[(j + 1) * SW + i + 1];

            double n00 = clamp(dotProduct(g00, Vector2d(u, v)));
            double n10 = clamp(dotProduct(g10, Vector2d(u - 1, v)));
            double n01 = clamp(dotProduct(g01, Vector2d(u, v - 1)));
            double n11 = clamp(dotProduct(g11, Vector2d(u - 1, v - 1)));

            double nx0 = n00 * (1 - mix(u)) + n10 * mix(u);
            double nx1 = n01 * (1 - mix(u)) + n11 * mix(u);
            double nxy = nx0 * (1 - mix(v)) + nx1 * mix(v);

            auto val = sf::Uint8(nxy * 128 + 128);

            if (mix(u) > 1)
                std::cerr << "Bad ";

            tmp[0] = val;
            tmp[1] = 0;
            tmp[2] = val;
            tmp[3] = 255;
            tmp += 4;
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT),
                            "Perlin Noise Visualized", sf::Style::Close);

    // Randomly generate a few gradients
    std::array<Vector2d, SW * SH> gradients;
    makeGradients(gradients, 0);

    // Pixels 32-bit RGBA (each 8 bits in width)
    sf::Uint8 pixels[WIDTH * HEIGHT * 4];
    makePixels(pixels, gradients);

    // Image buffer to hold the everything
    sf::Image generated;
    generated.create(WIDTH, HEIGHT, pixels);

    // Texture to put into graphics card
    sf::Texture background;
    background.loadFromImage(generated);

    // Sprite to draw
    sf::Sprite bgSprite;
    bgSprite.setTexture(background);
    bgSprite.setPosition(0, 0);

    sf::Event evt;
    bool draw = true;
    while (window.isOpen())
    {
        while (window.pollEvent(evt))
        {
            switch (evt.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                draw = true;
                break;
            default:
                break;
            }
            if (evt.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        if (draw)
        {
            // (Re)draw when necessary
            window.clear();
            window.draw(bgSprite);
            draw = false;
        }

        window.display();
    }
    return 0;
}
