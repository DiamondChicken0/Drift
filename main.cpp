#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <math.h>
#include <filesystem>

using namespace std;
using namespace sf;

class CarPhysics
{
    public:

        bool accelerating = false;
        bool braking = false;
        bool turningLeft = false;
        bool turningRight = false;
        bool manual = true;
        bool startUp = false;
        bool running = false;

        int gearing = 1;
        const int maxGearing = 6;
        const int weight = 2000;

        double RPM;
        double speed = 0.0;
        double wheelAngle = 0.0;

        float turningSpeed = 0.02f;
        float acceleration = 0.01f;
        float deceleration = 0.02f;
        const float maxSpeed = 4.0f;
        const float maxReverse = -2.0f;
        const float maxWheelAngle = 1.0f;
        const float friction = 0.002f;

        Time lastGearChange;

        void accelerator()
        {
            if (running)
            {
                speed += acceleration;
                speed = (speed > 2.0) ? 2.0 : speed;
                RPM += 3;
            }

        }

        void brake()
        {
            if (running)
            {
                speed -= deceleration;
                speed = (speed < -1.0) ? -1.0 : speed;
            }
        }

        void wheelLeft()
        {
            wheelAngle -= turningSpeed;
        }

        void wheelRight()
        {
            wheelAngle += turningSpeed;
        }

        void gearUp()
        {
            if (manual && gearing < maxGearing)
            {
                gearing++;
                lastGearChange = seconds(0);
            }
        }

        void gearDown()
        {
            if (manual && gearing > 1)
            {
                gearing--;
            }
        }

        void applyPhysics(Sprite* car, Time timePerFrame)
        {
            if (startUp && RPM < 800) {
                RPM += 800 * timePerFrame.asSeconds();
                if (RPM >= 800)
                {
                    running = true;
                    startUp = false;
                }
            }
            if (accelerating)
                accelerator();
            if (braking)
                brake();
            if (turningLeft)
                wheelLeft();
            if (turningRight)
                wheelRight();
            if (!turningLeft && !turningRight && wheelAngle != 0)
                wheelAngle = (wheelAngle >= 0) ? wheelAngle - 0.01 : wheelAngle + 0.01;

            wheelAngle = (wheelAngle > maxWheelAngle) ? maxWheelAngle : wheelAngle;
            wheelAngle = (wheelAngle < -1*maxWheelAngle) ? -1 * maxWheelAngle : wheelAngle;

            car->move(speed * sin(car->getRotation() * (M_PI/180)), -1 * speed * cos((car->getRotation() * (M_PI/180))));
            car->setRotation(car->getRotation() + (wheelAngle * ((speed >= 0) ? speed/maxSpeed : 1 * speed/maxReverse)));

            if (speed > 0)
                speed -= friction;
            else if (speed < 0)
                speed += friction;
            else
                speed = 0;
        }

};

class uiElements
{
    public:

    //speedometer = sm
    RenderWindow smFinal;
    Texture smTexture;
    Sprite smSprite;
    Font smFont;

    RectangleShape smRPM_BG;
    CircleShape smDP;
    CircleShape smRPMGauge;
    ConvexShape smBoundingTriL;
    ConvexShape smBoundingTriR;

    // R -> L
    Text smRPMText0;
    Text smRPMText1;
    Text smRPMText2;
    Text smRPMText3;

    Text smSpeedText;
    Text smImperialText;

    uiElements()
    {
        smFont.loadFromFile(filesystem::path(__FILE__).parent_path().string() + "/Open 24 Display St.ttf.");

        //smBG.setRadius(125);
        //smBG.setPosition(1620,780);
        //smBG.setFillColor(Color::Black);

        smDP.setRadius(110);
        smDP.setOrigin(smDP.getLocalBounds().width/2, smDP.getLocalBounds().height/2);
        smDP.setPosition(1635,795);
        smDP.setFillColor(Color{Color::White});
        smDP.setOutlineColor(Color::Black);
        smDP.setOutlineThickness(8);

    }

    void updateSpeedometer(double RPM, double speed, float maxSpeed)
    {
        try
        {
            int i = to_string((int)RPM).length();
            smRPMText0.setString(to_string((int)RPM)[i-1]);
            smRPMText1.setString(to_string((int)RPM)[i-2]);
            smRPMText2.setString(to_string((int)RPM)[i-3]);
            smRPMText3.setString(to_string((int)RPM)[i-4]);
        }
        catch (exception e){
            ;
        }

        //0100 should be 100
        if (smRPMText3.getString() == "0")
        {
            smRPMText3.setString("");
            if (smRPMText2.getString() == "0")
            {
                smRPMText2.setString("");
                if (smRPMText1.getString() == "0")
                {
                    smRPMText1.setString("");
                    if (smRPMText0.getString() == "0")
                    {
                        smRPMText3.setString("Z");
                        smRPMText2.setString("E");
                        smRPMText1.setString("R");
                        smRPMText0.setString("o");
                    }
                }
            }
        }
    }

    void drawAllUI(RenderWindow* window)
    {

        window->draw(smDP);
        window->draw(smRPM_BG);
        //window->draw(smRPMText);
    }
};

enum Controls
{
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_LEFT,
    MOVE_RIGHT,
    HANDBRAKE,
    PAUSE,
    GEAR_UP,
    GEAR_DOWN
};

unordered_map<Controls, Keyboard::Key> ControlMap =
        {
                {
                        {MOVE_FORWARD, Keyboard::W},
                        {MOVE_BACKWARD, Keyboard::S},
                        {MOVE_LEFT, Keyboard::A},
                        {MOVE_RIGHT, Keyboard::D},
                        {HANDBRAKE, Keyboard::Space},
                        {PAUSE, Keyboard::Escape},
                        {GEAR_UP, Keyboard::Up},
                        {GEAR_DOWN, Keyboard::Down}
                }
        };

int main() {

    //Clock
    Clock clock;

    const int fps = 144;
    const Time timePerFrame = seconds(1.0/fps);

    View gameView(FloatRect(0,0,1920,1080));
    ContextSettings settings;
    settings.antialiasingLevel = 16;
    RenderWindow window(VideoMode(1920,1080), "Drift", Style::Default, settings);

    Texture carImg;
    carImg.loadFromFile(filesystem::path(__FILE__).parent_path().string() + "/car.png");

    uiElements uiSprites;

    Sprite car;
    car.setTexture(carImg);
    car.setPosition(200,200);
    car.setOrigin(100,50);

    CarPhysics physics;

    while (window.isOpen())
    {
        clock.restart();
        Event event;
        while (window.pollEvent(event))
        {
            switch (event.type) {

                case (Event::Closed): {
                    window.close();
                    break;
                }

                /*
                 * ANYTHING THAT IS A CONTINUOUS ACTION SHOULD BE WRITTEN AS A BOOLEAN
                 * IT WILL NOT BE TIMED CORRECTLY OTHERWISE
                 */
                case (Event::KeyPressed): {
                    if (Keyboard::isKeyPressed(ControlMap[MOVE_FORWARD])) {
                        physics.accelerating = true;
                        physics.startUp = true;
                    }
                    if (Keyboard::isKeyPressed(ControlMap[MOVE_BACKWARD])) {
                        physics.braking = true;
                    }
                    if (Keyboard::isKeyPressed(ControlMap[MOVE_LEFT])) {
                        physics.turningLeft = true;
                    }
                    if (Keyboard::isKeyPressed(ControlMap[MOVE_RIGHT])) {
                        physics.turningRight = true;
                    }
                    if (Keyboard::isKeyPressed(ControlMap[HANDBRAKE])) {

                    }
                    if (Keyboard::isKeyPressed(ControlMap[PAUSE])) {

                    }
                    if (Keyboard::isKeyPressed(ControlMap[GEAR_UP])) {
                        physics.gearUp();
                    }
                    if (Keyboard::isKeyPressed(ControlMap[GEAR_DOWN])) {
                        physics.gearDown();
                    }
                    break;
                }

                case (Event::KeyReleased): {
                    if (event.key.code == ControlMap[MOVE_FORWARD]) {
                        physics.accelerating = false;
                    }
                    if (event.key.code == ControlMap[MOVE_BACKWARD]) {
                        physics.braking = false;
                    }
                    if (event.key.code == ControlMap[MOVE_LEFT]) {
                        physics.turningLeft = false;
                    }
                    if (event.key.code == ControlMap[MOVE_RIGHT]) {
                        physics.turningRight = false;
                    }
                    if (event.key.code == ControlMap[HANDBRAKE]) {
                    }
                    break;
                }
            }
        }

        /*
         * things that need to be updated, redrawn, recalculated each frame
         */

        physics.applyPhysics(&car, timePerFrame);
        uiSprites.updateSpeedometer(physics.RPM, physics.speed, physics.maxSpeed);
        gameView.setCenter(car.getPosition().x, car.getPosition().y);
        //window.setView(gameView);


        window.clear(Color::White);
        window.draw(car);

        uiSprites.drawAllUI(&window);
        window.display();

        sleep(timePerFrame - clock.getElapsedTime());
    }
    return 0;
}
