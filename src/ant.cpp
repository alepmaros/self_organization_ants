/*
 * Alexandre Maros - 2017/1
 * Ant Clustering Algorithm
 *
 */

#include "ant.h"
#include <iostream>
#include <cstdlib>

Ant::Ant(bool isDead, sf::Vector2i position, int radius, Ant*** deadAntGrid,
        Ant*** aliveAntGrid, int antSize, int gridSize)
{
    this->mIsDead = isDead;
    this->mGridPosition = position;
    this->mPosition = sf::Vector2f(position.x * antSize, position.y * antSize);;
    this->mRadius = radius;
    this->deadAntGrid = deadAntGrid;
    this->aliveAntGrid = aliveAntGrid;
    this->mCurrentStatus = Status::Moving;
    this->mAntSize = antSize;
    this->mGridSize = gridSize;

    // This is how many cells the ant will see with radius x
    // Which is the sum from 1 to radius of 8 times i
    this->mCellsSeen = 0;
    for (int i = 1; i <= radius; i++)
    {
        this->mCellsSeen += 8 * i;
    }

    if (isDead)
    {
        this->mColor = sf::Color::Red;
    }
    else
    {
        this->mColor = sf::Color::Black;
    }

    // making the body
    this->mBody.setSize(sf::Vector2f(5,5));
    this->mBody.setFillColor(this->mColor);
    this->mBody.setPosition(this->mPosition);
}



void Ant::draw(sf::RenderWindow *window)
{
    //while(window->isOpen())
    //{
    window->draw(this->mBody); 

    //}
}

// Only alive ants should call this.
void Ant::update()
{
    if (deadAntGrid[mGridPosition.y][mGridPosition.x])
    {
        deadAntGrid[mGridPosition.y][mGridPosition.x]->move(sf::Vector2f(mGridPosition.x, mGridPosition.y));
    }
    if (this->mCurrentStatus == Status::Moving)
    {
        // Find another direction to move to
        // Check piles etc
        //std::cout << "There is x dead ants around me: " << deadAntCount << std::endl;

        if (deadAntGrid[mGridPosition.y][mGridPosition.x])
        {
            int deadAntCount = this->countDeadAnts();
            // How the chance of carrying an ant is calculated:
            //  The ant will carry a dead ant if it thinks that ant does not belong to a pile.
            //  To get that chance you just divide the emptyCells around the ant by the total
            //  number of cells.
            int chanceOfCarryingAnt = ((mCellsSeen - deadAntCount) / ((double) mCellsSeen)) * 100;
            //std::cout << "There is a x% chance of carrying this ant: " << chanceOfCarryingAnt << std::endl;

            if( (std::rand() % 101) <= chanceOfCarryingAnt )
            {
                //std::cout << "I am starting to carry an ant at position " << 
                    //mGridPosition.y << " - " << mGridPosition.x << std::endl;
                this->mCurrentStatus = Status::Carrying;
            } 

        }
    }
    else
    {
        if (deadAntGrid[mGridPosition.y][mGridPosition.x])
        {
            int deadAntCount = this->countDeadAnts();

            // Explain this
            int chanceOfDroppingAnt = (int) ((1.0 - (deadAntCount / ((double) mCellsSeen))) * 100);
            //std::cout << mCellsSeen << "-" << deadAntCount << "-" << chanceOfDroppingAnt << std::endl;
            if ( (std::rand() % 101) <= chanceOfDroppingAnt )
            {
                //std::cout << "I dropped ant" << std::endl;
                this->mCurrentStatus = Status::Moving;
            }
        }
    }

    // Perform Movement

    Direction whereToMove = static_cast<Direction>((std::rand() % 4));
    int nextPosY = mGridPosition.y;
    int nextPosX = mGridPosition.x;
    switch(whereToMove)
    {
        case Direction::North:
            nextPosY--;
            break;
        case Direction::South:
            nextPosY++;
            break;
        case Direction::East:
            nextPosX++;
            break;
        case Direction::West:
            nextPosX--;
            break;
    }

    // If next movement is invalid
    if (nextPosY < 0 || nextPosY >= mGridSize || nextPosX < 0 || nextPosX >= mGridSize)
    {
        return;
    }

    //std::cout << "Movement: " << nextPosX << "-" << nextPosY << std::endl;

    // If there is an Alive ant in the next position, do nothing.
    if (aliveAntGrid[nextPosY][nextPosX])
    {
        return;
    }
    if (this->mCurrentStatus == Status::Carrying)
    {
        // If I am carrying a dead ant and there is a dead ant in the next space
        // do nothing.
        if (deadAntGrid[nextPosY][nextPosX])
        {
            return;
        }
    }
    //std::cout << "2Movement: " << nextPosX << "-" << nextPosY << std::endl;
    // When implementing with multithread, lock this area.
    aliveAntGrid[nextPosY][nextPosX] = aliveAntGrid[mGridPosition.y][mGridPosition.x];
    aliveAntGrid[mGridPosition.y][mGridPosition.x] = nullptr;
    //std::cout << "3Movement: " << nextPosX << "-" << nextPosY << std::endl;
    if (this->mCurrentStatus == Status::Carrying)
    {
        //std::cout << "4Movement: " << nextPosX << "-" << nextPosY << std::endl;
        deadAntGrid[nextPosY][nextPosX] = deadAntGrid[mGridPosition.y][mGridPosition.x];
        //std::cout << "4Movement: " << nextPosX << "-" << nextPosY << std::endl;
        deadAntGrid[mGridPosition.y][mGridPosition.x] = nullptr;
        //std::cout << "4Movement: " << nextPosX << "-" << nextPosY << std::endl;
        deadAntGrid[nextPosY][nextPosX]->move(sf::Vector2f(nextPosX,nextPosY));
        //std::cout << "5Movement: " << nextPosX << "-" << nextPosY << std::endl;
    }

    this->move(sf::Vector2f(nextPosX,nextPosY));
    //std::cout << "6Movement: " << nextPosX << "-" << nextPosY << std::endl;
}

void Ant::move(sf::Vector2f nextPos)
{
    mGridPosition.x = (int) nextPos.x;
    mGridPosition.y = (int) nextPos.y;
    mBody.setPosition(sf::Vector2f(nextPos.x * mAntSize, nextPos.y * mAntSize));
}

int Ant::countDeadAnts()
{
    int deadAntCount = 0;
    int posX, posY;
    for (int i = -mRadius; i <= mRadius; i++)
    {
        for (int j = -mRadius; j <= mRadius; j++) 
        {
            posY = mGridPosition.y + i;
            posX = mGridPosition.x + j;
            if (posY >= 0 && posX >= 0 && posY < mGridSize && posX < mGridSize)
            {
                if (deadAntGrid[posY][posX] != nullptr)
                {
                    deadAntCount++;
                }
            }
        }
    }
    return deadAntCount;
}
