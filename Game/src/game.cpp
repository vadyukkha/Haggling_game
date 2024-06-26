// Copyright 2024 by vadyukkha
#include <iostream>
#include <dlfcn.h>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <string>
#include <fstream>

typedef int(*DecideFunc)(int, int, int, int, int, int, int);

void output_picked_offer(const std::vector<int>& v, std::ofstream& result_file) {
    result_file << "Item costs: ";
    for (const auto it : v) {
        result_file << it << " ";
    }
    result_file << std::endl;
} 

const std::vector<std::vector<int>> possible_offers = {
    {10, 0, 0}, {8, 1, 0}, {7, 0, 1}, {6, 2, 0}, {5, 1, 1}, {4, 0, 2},
    {4, 3, 0}, {3, 2, 1}, {2, 4, 0}, {2, 1, 2}, {1, 0, 3}, {1, 3, 1},
    {0, 2, 2}, {0, 5, 0}
};

int evaluate_offer(const std::vector<int>& offer, int hat_cost, int book_cost, int ball_cost) {
    return offer[0] * hat_cost + offer[1] * book_cost + offer[2] * ball_cost;
}

int player_points = 0;
int opponent_points = 0;
int games_played = 0;
int player_wins = 0;
int opponent_wins = 0;
int draws = 0;

int i = 0;
int j = 0;

void play_game196(void* handleA, void* handleB, std::ofstream& result_file, const std::string& user1, const std::string& user2) {
    // загружаем функции
    DecideFunc decideA = (DecideFunc)dlsym(handleA, "decide");
    DecideFunc decideB = (DecideFunc)dlsym(handleB, "decide");

    if (!decideA || !decideB) {
        result_file << "Failed to get function address.\n";
        return;
    }

    int rand_A = i;
    int rand_B = j;

    // выбираем из все возможных сумм одну комбинацию для каждого игрока
    output_picked_offer(possible_offers[rand_A], result_file);
    output_picked_offer(possible_offers[rand_B], result_file);
    int hatCostA = possible_offers[rand_A][0], bookCostA = possible_offers[rand_A][1], ballCostA = possible_offers[rand_A][2];
    int hatCostB = possible_offers[rand_B][0], bookCostB = possible_offers[rand_B][1], ballCostB = possible_offers[rand_B][2];

    // подсчет поинтов игроков
    int playerA_points = 0, playerB_points = 0;

    int hatCount = 0, bookCount = 0, ballCount = 0;
    for (int turn = 0; turn < 6; ++turn) {
        if (turn % 2 == 0) {
            // Ход игрока A
            int decision = decideA(turn, hatCostA, bookCostA, ballCostA, hatCount, bookCount, ballCount);
            if (decision == 200) {
                result_file << "Player " << user1 << " accepted the offer.\n";
                
                // Оценка очков игрока A за этот раунд
                int my_value = evaluate_offer({hatCount, bookCount, ballCount}, hatCostA, bookCostA, ballCostA);
                int enemy_value = evaluate_offer({1 - hatCount, 2 - bookCount, 3 - ballCount}, hatCostB, bookCostB, ballCostB);
                playerA_points += my_value;
                playerB_points += enemy_value;

                break;
            } else {
                hatCount = decision / 100;
                bookCount = (decision / 10) % 10;
                ballCount = decision % 10;
                result_file << "Player " << user1 << " proposes: " << hatCount << " hats, " << bookCount << " books, " << ballCount << " balls.\n";
            }
        } else {
            // Ход игрока B
            int decision = decideB(turn, hatCostB, bookCostB, ballCostB, hatCount, bookCount, ballCount);
            if (decision == 200) {
                result_file << "Player " << user2 << " accepted the offer.\n";

                // Оценка очков игрока B за этот раунд
                int my_value = evaluate_offer({hatCount, bookCount, ballCount}, hatCostB, bookCostB, ballCostB);
                int enemy_value = evaluate_offer({1 - hatCount, 2 - bookCount, 3 - ballCount}, hatCostA, bookCostA, ballCostA);
                playerB_points += my_value;
                playerA_points += enemy_value;

                break;
            } else {
                hatCount = decision / 100;
                bookCount = (decision / 10) % 10;
                ballCount = decision % 10;
                result_file << "Player " << user2 << " proposes: " << hatCount << " hats, " << bookCount << " books, " << ballCount << " balls.\n";
            }
        }
    }

    // Вывод результатов после каждой игры
    result_file << "\nGame " << games_played << " Results:" << std::endl;
    result_file << "Player " << user1 << " Points: " << playerA_points << std::endl;
    result_file << "Player " << user2 << " Points: " << playerB_points << std::endl;
    if (playerA_points > playerB_points) {
        result_file << "Player " << user1 << " wins this game." << std::endl << std::endl;
        player_wins += 1;
    } else if (playerA_points < playerB_points) {
        result_file << "Player " << user2 << " wins this game." << std::endl << std::endl;
        opponent_wins += 1;
    } else {
        result_file << "It's a draw." << std::endl << std::endl;
        draws += 1;
    }

    player_points += playerA_points;
    opponent_points += playerB_points;

    games_played += 1;
    j++;
    if (j == 14) {
        i++;
        j = 0;
    }
}

int main() {
    srand(time(0));
    std::ifstream t_name("teamname.txt");
    if (!t_name.is_open()) {
        std::cerr << "Failed to open teamname.\n";
        return 1;
    }
    
    std::string TEAM_NAME;
    t_name >> TEAM_NAME;

    std::string output = "../bd/result_" + TEAM_NAME + ".txt";
    std::ofstream result_file(output);

    if (!result_file.is_open()) {
        std::cerr << "Failed to open result file.\n";
        return 1;
    }

    std::string user_folder_link = "../Users/" + TEAM_NAME + "/" + TEAM_NAME + ".dylib";
    std::string bot_hat_lover = "../Users/hat_lover/hat_lover.dylib";
    std::string bot_book_lover = "../Users/book_lover/book_lover.dylib";
    std::string bot_ball_lover = "../Users/ball_lover/ball_lover.dylib";

    std::vector<std::string> bots = {bot_hat_lover, bot_book_lover, bot_ball_lover};
    std::vector<std::string> names = {"bot_hat_lover", "bot_book_lover", "bot_ball_lover"};

    for (int count = 0; count < bots.size(); count++) {
        result_file << "---------- " << names[count] << " ----------" << std::endl;
        void* handleA = dlopen(user_folder_link.c_str(), RTLD_LAZY);
        void* handleB = dlopen(bots[count].c_str(), RTLD_LAZY);

        if (!handleA || !handleB) {
            result_file << "Failed to load shared libraries.\n";
            return 1;
        }

        for (int c = 0; c < 196; c++) {
            play_game196(handleA, handleB, result_file, TEAM_NAME, names[count]);
        }

        int games_played_overall = games_played;
        int player_wins_overall = player_wins;
        int opponent_wins_overall = opponent_wins;
        int draws_overall = draws;
        int player_points_overall = player_points;
        int opponent_points_overall = opponent_points;

        result_file << "---------- swap of sides ----------" << std::endl;

        player_points = 0;
        opponent_points = 0;
        games_played = 0;
        player_wins = 0;
        opponent_wins = 0;
        draws = 0;

        i = 0;
        j = 0;

        for (int c = 0; c < 196; c++) {
            play_game196(handleB, handleA, result_file, names[count], TEAM_NAME);
        }

        games_played_overall += games_played;
        player_wins_overall += opponent_wins;
        opponent_wins_overall += player_wins;
        draws_overall += draws;
        player_points_overall += opponent_points;
        opponent_points_overall += player_points;

        dlclose(handleA);
        dlclose(handleB);

        player_points = 0;
        opponent_points = 0;
        games_played = 0;
        player_wins = 0;
        opponent_wins = 0;
        draws = 0;

        i = 0;
        j = 0;

        // Вывод общих результатов
        result_file << "Overall Results:" << std::endl;
        result_file << "Games Played: " << games_played_overall << std::endl;
        result_file << "Player " << TEAM_NAME << " Wins: " << player_wins_overall << std::endl;
        result_file << "Player " << names[count] << " Wins: " << opponent_wins_overall << std::endl;
        result_file << "Draws: " << draws_overall << std::endl;
        result_file << "Total Player " << TEAM_NAME << " Points: " << player_points_overall << std::endl;
        result_file << "Total Player " << names[count] << " Points: " << opponent_points_overall << std::endl;
        result_file << "Player " << TEAM_NAME << " percent of points: " << (double)player_points_overall / (double)(10 * games_played_overall)  * 100 << std::endl;
        result_file << "Player " << names[count] << " percent of points: " << (double)opponent_points_overall / (double)(10 * games_played_overall) * 100 << std::endl << std::endl;
    }

    result_file.close();

    return 0;
}
