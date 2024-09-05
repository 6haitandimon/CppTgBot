#include <iostream>
#include <unordered_map>
#include <tgbot/tgbot.h>
#include <mysqlx/xdevapi.h>
#include <sw/redis++/redis++.h>
#include "StateMachine.h"
#include "User.h"
#include "Task.h"
#include "TOKEN.h"

//sw::redis::Redis redis("127.0.0.1:6379");

void CreateKayboard(const std::vector<std::pair<std::string, std::string>> buttonStrings, TgBot::InlineKeyboardMarkup::Ptr& kb){
    for(auto iter: buttonStrings){
        std::vector<TgBot::InlineKeyboardButton::Ptr> row;
        TgBot::InlineKeyboardButton::Ptr button(new TgBot::InlineKeyboardButton);
        button->text = iter.first;
        button->callbackData = iter.second;
        row.push_back(button);
        kb->inlineKeyboard.push_back(row);
    }
}

void loadUserData(mysqlx::Table& table, std::unordered_map<int64_t, USER::User>& userInfos,
                  std::unordered_map<int64_t, StateMachine::StateMachine> &stateMachine) {
    try {
        mysqlx::RowResult rowData = table.select("*").execute();

        for(auto index : rowData){
            stateMachine[index[0]] = StateMachine::StateMachine(StateMachine::StartState::START);
            userInfos[index[0]] = USER::User(static_cast<int>(index[0]), static_cast<std::string>(index[1]), static_cast<int>(index[2]), static_cast<int>(index[3]));
        }

    }catch (const mysqlx::Error &err) {
        std::cerr << "Error: " << err.what() << std::endl;
    }
}



void StartWork(){

}

void StartRegistration(TgBot::Bot &bot, TgBot::Message::Ptr &message,
                       std::unordered_map<int64_t, StateMachine::StateMachine> &stateMachine){

    TgBot::InlineKeyboardMarkup::Ptr keyboardRegister(new TgBot::InlineKeyboardMarkup);
    CreateKayboard({std::make_pair("Регистрация", "register")}, keyboardRegister);

    bot.getApi().sendMessage(message->chat->id, "Привет! Вас приветствует бот Robbo Club(Минск) для сотрудников");
    bot.getApi().sendMessage(message->chat->id, "Выберете действие:", nullptr, nullptr, keyboardRegister);

}

void CommandRegistration(TgBot::Bot &bot, TgBot::Message::Ptr &message, std::unordered_map<int64_t, USER::User>
        &userInfos, std::unordered_map<int64_t, StateMachine::StateMachine> &stateMachine){
    int64_t chatId = message->chat->id;

    if(userInfos.find(chatId) != userInfos.end() && userInfos[chatId].GetPosition() != 0){
        StartWork();
    }else {
        stateMachine[chatId] = StateMachine::StateMachine(StateMachine::RegistrationState::START_REGISTRATION);
        StartRegistration(bot, message, stateMachine);
    }

}

void CallBack(TgBot::Bot &bot, TgBot::CallbackQuery::Ptr& query,
              std::unordered_map<int64_t, USER::User> &userInfos,
              std::unordered_map<int64_t, StateMachine::StateMachine> &stateMachine){
    int64_t chatId = query->message->chat->id;
    if(query->data == "register"
            || query->data == "teacher"
            || query->data == "admin"
            || query->data == "seo"
            || query->data == "tech") {
            std::visit([&bot, &query, &stateMachine, &userInfos, &chatId](auto &&arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, StateMachine::RegistrationState>) {
                    switch (arg) {
                        case StateMachine::RegistrationState::START_REGISTRATION: {
                            stateMachine[chatId].SetState(StateMachine::RegistrationState::GET_NAME);
                            bot.getApi().sendMessage(chatId, "Пожалуйста введите ваше имя");
                            userInfos[chatId] = USER::User(chatId);
                            break;
                        }
                        case StateMachine::RegistrationState::GET_ROLE: {
                            stateMachine[chatId].SetState(StateMachine::RegistrationState::GET_CLUB);
//                            TgBot::InlineKeyboardMarkup::Ptr selectPosition(new TgBot::InlineKeyboardMarkup);
//                            CreateKayboard({std::make_pair("Преподаватель", "teacher"),
//                                            std::make_pair("Администратор", "admin"),
//                                            std::make_pair("Директор", "seo"),
//                                            std::make_pair("Инженер", "tech")}, selectPosition);
                            if(query->message->messageId)
                                bot.getApi().deleteMessage(chatId, query->message->messageId);
                            TgBot::InlineKeyboardMarkup::Ptr selectClub(new TgBot::InlineKeyboardMarkup);
                            CreateKayboard({std::make_pair("ТЦ Титан", "titan"), std::make_pair("Ул. Мстиславца", "mstislavca"), std::make_pair("Боровляны", "borovlyani")}, selectClub);
                            if (query->data == "teacher") {
                                    userInfos[chatId].SetPosition(Teacher);
                            } else if (query->data == "admin") {
                                    userInfos[chatId].SetPosition(Admin);
                            } else if (query->data == "tech") {
                                    userInfos[chatId].SetPosition(Tech);
                            } else if (query->data == "seo") {
                                    userInfos[chatId].SetPosition(SEO);
                            }

                        }
                    }
                }
            },stateMachine[chatId].GetState());
    }
}

int main() {
    std::unordered_map<int64_t, USER::User> userInfos;
    std::unordered_map<int64_t, StateMachine::StateMachine> stateMachine;

    TgBot::Bot bot(TOKEN::token);

    mysqlx::Session mysql("localhost", 33060, "root", "root");
    mysqlx::Schema schema = mysql.getSchema("Robbo");
    mysqlx::Table table = schema.getTable("users");

    loadUserData(table, userInfos, stateMachine);


    bot.getEvents().onCommand("start", [&bot, &userInfos, &stateMachine](TgBot::Message::Ptr message) -> void{
        CommandRegistration(bot, message, userInfos, stateMachine);
    });
//        int64_t chatId = message->chat->id;
//        stateMachine[chatId].SetState(StateMachine::StartState::START);
//        printf("User started: %s\n", message->chat->username.c_str());
//        if(userInfos.find(chatId) != userInfos.end() && userInfos[chatId].GetPosition() != 0){
//
//        }
//        else{
//            TgBot::InlineKeyboardMarkup::Ptr keyboardRegister(new TgBot::InlineKeyboardMarkup);
//            CreateKayboard({std::make_pair("Регистрация", "register")}, keyboardRegister);
//
//            bot.getApi().sendMessage(message->chat->id, "Привет! Вас приветствует бот Robbo Club(Минск) для сотрудников");
//            bot.getApi().sendMessage(message->chat->id, "Выберете действие:", nullptr, nullptr, keyboardRegister);
//        }
//
//    });
//
//    bot.getEvents().onCommand("delete", [&bot, &table](TgBot::Message::Ptr message){
//        int64_t chatId = message->chat->id;
//        if(userInfos.find(chatId) != userInfos.end() && userInfos[chatId].GetPosition() != 0){
//            userInfos.erase(userInfos.find(chatId));
//
//            try{
//                table.remove().where("chat_id = :id")
//                        .bind("id", chatId)
//                        .execute();
////                SQLite::Statement query(db, "DELETE FROM users WHERE chat_id = ?");
////                query.bind(1, chatId);
////                query.exec();
//
//                bot.getApi().sendMessage(chatId, "Данные успешно удалены!");
//                bot.getApi().sendMessage(chatId, "Для повторной регистрации воспользуейтесь командой: /start");
//            }catch(const std::exception &e) {
//                std::cerr << "Exception: " << e.what() << std::endl;
//                bot.getApi().sendMessage(chatId, "Ошибка удаления, ообратитесь в поддержку: @againTL");
//            }
//        }else{
//            bot.getApi().sendMessage(chatId, "Ваши данные не обнаружены!");
//        }
//    });
//    bot.getEvents().onCommand("work", [&bot](TgBot::Message::Ptr message){
//        int64_t chatId = message->chat->id;
//        if(userInfos.find(chatId) != userInfos.end() && userInfos[chatId].GetPosition() != 0) {
//            TgBot::InlineKeyboardMarkup::Ptr selectAction(new TgBot::InlineKeyboardMarkup);
//            CreateKayboard({std::make_pair("Создать задачу", "createTask"), std::make_pair("Cписок задач", "taskList")}, selectAction);
//
//            bot.getApi().sendMessage(message->chat->id, "Выберете действие:", nullptr, nullptr, selectAction);
//        }else{
//            bot.getApi().sendMessage(message->chat->id, "Вы не зарегестрированы в системе!");
//            bot.getApi().sendMessage(message->chat->id, "Для регистрации выполните нажмите: /start");
//        }
//    });
    bot.getEvents().onCallbackQuery([&bot, &userInfos, &stateMachine](TgBot::CallbackQuery::Ptr query) {
        CallBack(bot, query, userInfos, stateMachine);
    });
//        if (StringTools::startsWith(query->data, "register")) {
//
//            int64_t chatId = query->message->chat->id;
//            if(query->message->messageId && query->message->messageId - 1){
//                bot.getApi().deleteMessage(chatId, query->message->messageId);
//                bot.getApi().deleteMessage(chatId, query->message->messageId - 1);
//            }
//
//            if(userInfos.find(chatId) == userInfos.end() || userInfos[chatId].GetName().empty()){
//                bot.getApi().sendMessage(chatId, "Пожалуйста введите ваше имя");
//                userInfos[chatId] = USER::User(chatId);
//
//            }
//        }
//    });
//    bot.getEvents().onNonCommandMessage([&bot](TgBot::Message::Ptr message) {
//        int64_t chatId = message->chat->id;
//
//        if (userInfos.find(chatId) != userInfos.end() && userInfos[chatId].GetName().empty()) {
//
//            userInfos[chatId].SetName(message->text);
//            if(message->messageId && message->messageId - 1){
//                bot.getApi().deleteMessage(chatId, message->messageId);
//                bot.getApi().deleteMessage(chatId, message->messageId - 1);
//            }
//            TgBot::InlineKeyboardMarkup::Ptr selectPosition(new TgBot::InlineKeyboardMarkup);
//            CreateKayboard({std::make_pair("Преподаватель", "teacher"), std::make_pair("Администратор", "admin"), std::make_pair("Директор", "seo"), std::make_pair("Инженер", "tech")}, selectPosition);
//
//
//
//            bot.getApi().sendMessage(chatId, userInfos[chatId].GetName() + ", выберете вашу должность:", nullptr, nullptr, selectPosition);
//
//
//        }
//    });
//
//
//    bot.getEvents().onCallbackQuery([&bot](TgBot::CallbackQuery::Ptr query) {
//        int64_t chatId = query->message->chat->id;
//
//        if (userInfos.find(chatId) != userInfos.end() && !userInfos[chatId].GetPosition()) {
//            if (query->data == "teacher" || query->data == "admin" || query->data == "seo" || query->data == "tech") {
//                if(query->message->messageId)
//                    bot.getApi().deleteMessage(chatId, query->message->messageId);
//                TgBot::InlineKeyboardMarkup::Ptr selectClub(new TgBot::InlineKeyboardMarkup);
//                CreateKayboard({std::make_pair("ТЦ Титан", "titan"), std::make_pair("Ул. Мстиславца", "mstislavca"), std::make_pair("Боровляны", "borovlyani")}, selectClub);
//                if (query->data == "teacher") {
//                    userInfos[chatId].SetPosition(Teacher);
//                } else if (query->data == "admin") {
//                    userInfos[chatId].SetPosition(Admin);
//                } else if (query->data == "tech") {
//                    userInfos[chatId].SetPosition(Tech);
//                } else if (query->data == "seo") {
//                    userInfos[chatId].SetPosition(SEO);
//                }
//
//
//                bot.getApi().sendMessage(chatId, userInfos[chatId].GetName() + ", выберете ваш клуб:", nullptr, nullptr, selectClub);
//            }
//
//        }
//    });
//
//    bot.getEvents().onCallbackQuery([&bot, &table](TgBot::CallbackQuery::Ptr query) {
//        int64_t chatId = query->message->chat->id;
//        if (userInfos.find(chatId) != userInfos.end() && !userInfos[chatId].GetClub()) {
//
//            if (query->data == "titan" || query->data == "mstislavca" || query->data == "borovlyani") {
//                if(query->message->messageId)
//                    bot.getApi().deleteMessage(chatId, query->message->messageId);
//                if (query->data == "titan") {
//                    userInfos[chatId].SetClub(Titan);
//                } else if (query->data == "mstislavca") {
//                    userInfos[chatId].SetClub(Mstislavca);
//                } else if (query->data == "borovlyani") {
//                    userInfos[chatId].SetClub(Borovlyani);
//                }
//
//                try {
//                    table.insert("chat_id", "name", "position", "club")
//                            .values(chatId, userInfos[chatId].GetName(),
//                                    userInfos[chatId].GetPosition(),
//                                    userInfos[chatId].GetClub())
//                            .execute();
//                    TgBot::InlineKeyboardMarkup::Ptr selectAction(new TgBot::InlineKeyboardMarkup);
//                    CreateKayboard({std::make_pair("Создать задачу", "createTask"), std::make_pair("Cписок задач", "taskList")}, selectAction);
//                    bot.getApi().sendMessage(chatId, "Информация успешно сохранена");
//                    bot.getApi().sendMessage(chatId, "Выберете действие:", nullptr, nullptr, selectAction);
//                } catch (const std::exception &e) {
//                    std::cerr << "Exception: " << e.what() << std::endl;
//                    bot.getApi().sendMessage(chatId, "Ошибка сохранения, ообратитесь в поддержку: @againTL");
//                }
//            }
//        }
//    });
//    try {
//        bot.getApi().deleteWebhook();
//        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
//        TgBot::TgLongPoll longPoll(bot);
//        while (true) {
//            longPoll.start();
//        }
//    } catch (TgBot::TgException& e) {
//        printf("error: %s\n", e.what());
////        longPoll.start();
//    }
    return 0;
}
/*
 * try {
SQLite::Statement query(db, "INSERT INTO users (chat_id, name, position, club) VALUES (?, ?, ?, ?)");
query.bind(1, static_cast<int>(chatId));
query.bind(2, userInfos[chatId].GetName());
query.bind(3, userInfos[chatId].GetPosition());
query.bind(4, userInfos[chatId].GetClub());
query.exec();

bot.getApi().sendMessage(chatId, "Информация успешно сохранена");
} catch (const std::exception &e) {
std::cerr << "Exception: " << e.what() << std::endl;
bot.getApi().sendMessage(chatId, "Ошибка сохранения, ообратитесь в поддержку: @againTL");
}
 */