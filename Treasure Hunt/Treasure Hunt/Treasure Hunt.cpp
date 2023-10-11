#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <chrono>
#include <thread>
#include <Windows.h>

#undef max

constexpr char playerChar = 'P';
constexpr char goalChar = 'G';
constexpr char obstacleChar = '#';
constexpr char emptyChar = '.';
constexpr char itemChar = 'I';
constexpr char bombChar = 'B';

// ANSI escape codes for text colors
const std::string textColorDefault = "\033[0m"; // Reset to the default color
const std::string textColorCyan = "\033[36m"; // Cyan
const std::string textColorRed = "\033[31m"; // Red
const std::string textColorGreen = "\033[32m"; // Green
const std::string textColorRandom = "\033[38;5;"; // Random color

std::string generateRandomColor( ) {
    auto now = std::chrono::system_clock::now( );
    auto duration = now.time_since_epoch( );
    int seed = std::chrono::duration_cast< std::chrono::milliseconds >( duration ).count( );
    std::mt19937 gen( seed );

    return textColorRandom + std::to_string( 16 + ( gen( ) % 200 ) ) + "m";
}

void printGrid( const std::vector<std::vector<char>>& grid, int itemsCollected, const std::string& playerColor ) {
    std::cout << "\033[2J\033[H";

    const std::string infoText = playerColor + "P is the player." + textColorGreen + "\nG is the goal." + textColorRed + "\nB is a bomb." + textColorDefault + "\n# is an obstacle.";

    for ( const auto& row : grid ) {
        for ( char cell : row ) {
            std::string textColor;
            if ( cell == itemChar )
                textColor = textColorCyan;
            else if ( cell == bombChar )
                textColor = textColorRed;
            else if ( cell == playerChar )
                textColor = playerColor;
            else if ( cell == goalChar )
                textColor = textColorGreen;

            std::cout << textColor << cell << textColorDefault << ' ';
        }
        std::cout << std::endl;
    }

    std::cout << infoText << std::endl;

    std::cout << "Total items collected: " << itemsCollected << textColorDefault << std::endl;
}

int main( ) {
    SetConsoleTitle( TEXT( "Treasure Hunt" ) );

    std::random_device rd;
    std::mt19937 gen( rd( ) );

    char playAgain = 'Y';

    while ( playAgain == 'Y' || playAgain == 'y' ) {
        int gridSize = 0;
        while ( gridSize < 2 ) {
            std::cout << "\033[2J\033[H";
            std::cout << "Enter the grid size: ";

            while ( !( std::cin >> gridSize ) || gridSize < 2 ) {
                std::cout << "Invalid input. Please enter a positive integer value greater than or equal to 2: ";
                std::cin.clear( );
                std::cin.ignore( std::numeric_limits<std::streamsize>::max( ), '\n' );
            }

            std::cin.ignore( std::numeric_limits<std::streamsize>::max( ), '\n' );
        }

        std::vector<std::vector<char>> grid( gridSize, std::vector<char>( gridSize, emptyChar ) );

        int playerX = 0;
        int playerY = 0;
        grid[ playerY ][ playerX ] = playerChar;

        int goalX, goalY;

        do {
            goalX = std::uniform_int_distribution<>( 0, gridSize - 1 )( gen );
            goalY = std::uniform_int_distribution<>( 0, gridSize - 1 )( gen );
        } while ( grid[ goalY ][ goalX ] != emptyChar );

        grid[ goalY ][ goalX ] = goalChar;

        int numWalls = 0;
        int numBombs = 0;
        std::vector<std::pair<int, int>> bombPositions;

        for ( int y = 0; y < gridSize; ++y ) {
            for ( int x = 0; x < gridSize; ++x ) {
                int randomValue = std::uniform_int_distribution<>( 1, 5 )( gen );
                if ( randomValue == 1 ) {
                    grid[ y ][ x ] = obstacleChar;
                    numWalls++;
                }
                else if ( randomValue == 2 && numBombs < numWalls / 3 ) {
                    grid[ y ][ x ] = bombChar;
                    bombPositions.emplace_back( x, y );
                    numBombs++;
                }
            }
        }

        grid[ playerY ][ playerX ] = playerChar;

        int numItems = 10;
        for ( int i = 0; i < numItems; ++i ) {
            int itemX, itemY;
            do {
                itemX = std::uniform_int_distribution<>( 0, gridSize - 1 )( gen );
                itemY = std::uniform_int_distribution<>( 0, gridSize - 1 )( gen );
            } while ( ( itemX == playerX && itemY == playerY ) ||
                      ( itemX == goalX && itemY == goalY ) ||
                      std::find( bombPositions.begin( ), bombPositions.end( ), std::make_pair( itemX, itemY ) ) !=
                      bombPositions.end( ) );
            grid[ itemY ][ itemX ] = itemChar;
        }

        bool gameOver = false;
        int itemsCollected = 0;

        std::string playerColor = generateRandomColor( );

        while ( !gameOver ) {
            printGrid( grid, itemsCollected, playerColor );

            char move;
            std::cout << "Enter a move (W/A/S/D to move, Q to quit, R to restart): ";
            std::cin >> move;

            int newPlayerX = playerX;
            int newPlayerY = playerY;

            switch ( move ) {
                case 'W':
                case 'w':
                    newPlayerY--;
                    break;
                case 'A':
                case 'a':
                    newPlayerX--;
                    break;
                case 'S':
                case 's':
                    newPlayerY++;
                    break;
                case 'D':
                case 'd':
                    newPlayerX++;
                    break;
                case 'Q':
                case 'q':
                    char quitConfirm;
                    std::cout << "Are you sure you want to quit? (Y/N): ";
                    std::cin >> quitConfirm;
                    if ( quitConfirm == 'Y' || quitConfirm == 'y' ) {
                        return 0;
                    }
                    break;
                case 'R':
                case 'r':
                    playAgain = 'Y';
                    gameOver = true;
                    break;
                default:
                    std::cout << "Invalid move! Use W/A/S/D to move, Q to quit, or R to restart." << std::endl;
            }

            if ( newPlayerX >= 0 && newPlayerX < gridSize && newPlayerY >= 0 && newPlayerY < gridSize ) {
                char cell = grid[ newPlayerY ][ newPlayerX ];
                if ( cell != obstacleChar ) {
                    grid[ playerY ][ playerX ] = emptyChar;
                    playerX = newPlayerX;
                    playerY = newPlayerY;

                    if ( cell == itemChar ) {
                        itemsCollected++;
                        grid[ newPlayerY ][ newPlayerX ] = playerChar;
                    }
                    else if ( cell == bombChar ) {
                        gameOver = true;
                        std::cout << "Game over! You triggered a bomb and collected " << itemsCollected << " items." << std::endl;
                    }
                    else {
                        grid[ playerY ][ playerX ] = playerChar;
                    }

                    if ( playerX == goalX && playerY == goalY ) {
                        std::cout << "\033[2J\033[H";
                        std::cout << "Congratulations! You reached the goal with " << itemsCollected
                            << " items collected!" << std::endl;
                        std::cout << "Do you want to play again? (Y/N): ";
                        std::string playChoice;
                        std::cin >> playChoice;
                        if ( playChoice == "Y" || playChoice == "y" ) {
                            playerX = 0;
                            playerY = 0;
                            grid[ playerY ][ playerX ] = playerChar;
                            playAgain = 'Y';
                            break;
                        }
                        else {
                            return 0;
                        }
                    }
                }
                else {
                    std::cout << "Oops! You bumped into an obstacle." << std::endl;
                }
            }
            else {
                std::cout << "Invalid move! You can't go outside the grid." << std::endl;
            }

            std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
        }
    }

    return 0;
}
