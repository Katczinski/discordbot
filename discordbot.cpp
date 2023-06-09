// discordbot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define CURL_STATICLIB

#include <iostream>
#include <dpp/dpp.h>
#include "openai.hpp"

const std::string    BOT_TOKEN = "MTExNTU3MDc3MzIxODgyNDI5Mg.GSuoEi.ubEirywP1OXjlhveH5SVUFqkQMV5obtVRIn1UE";

int main() {
    setenv("OPENAI_API_KEY", "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", true);
    dpp::cluster bot(BOT_TOKEN);
    bot.intents |= dpp::i_message_content;

    openai::start();
    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
        for (auto mention : event.msg.mentions) {
            if (mention.first == bot.me) {
                std::string msg = event.msg.content;
                nlohmann::json array;
                array.push_back(nlohmann::json::object({ { "role", "user" }, { "content", msg } }));
                nlohmann::json request {    {"model", "gpt-3.5-turbo" },
                    { "messages", array },
                    { "temperature", 0.2 },
                };
                std::string response;
                try {
                    auto chat = openai::chat().create(request);
                    nlohmann::json json = nlohmann::json::parse(chat.dump(2));
                    response = json.at("choices").at(0).at("message").at("content");
                }
                catch (const std::exception& e) {
                    nlohmann::json json = nlohmann::json::parse(e.what());
                    response = json.at("message");
                }
                event.reply(response, true);
                return;
            }
        }
        });
    bot.on_slashcommand([&](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "image") {
            std::string description = std::get<std::string>(event.get_parameter("description"));

            nlohmann::json request {
                {"prompt", description },
                { "n", 2 },
                { "size", "512x512" },
            };

            event.thinking();
            std::string url;
            try {
                auto image = openai::image().create({
                    { "prompt", description },
                    { "n", 1 },
                    { "size", "512x512" }
                    }); // Using initializer lists
                description = "**" + description + "** ";
                description += event.command.get_issuing_user().get_mention();
                url = image["data"][0]["url"];
            }
            catch (const std::exception& e) {
                nlohmann::json json = nlohmann::json::parse(e.what());
                std::string response = json.at("message");
                event.edit_original_response(response);
                return;
            }
            bot.request(url, dpp::m_get, [&bot, description, url, event](const dpp::http_request_completion_t& httpRequestCompletion) {
                // create a message
                dpp::snowflake channel_id = event.command.get_channel().id;
                dpp::message msg(channel_id, description);

                // attach the image on success
                if (httpRequestCompletion.status == 200) {
                    int pos = url.find_first_of(".png");
                    std::string name = url.substr(0, pos) + ".png";
                    msg.add_file(name, httpRequestCompletion.body);
                }

                // send the message
                event.edit_original_response(msg);
                });
        }
        });
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            /* Create a new global command on ready event */
            dpp::slashcommand newcommand("image", "Generate image", bot.me.id);
            newcommand.add_option(dpp::command_option(dpp::co_string, "description", "Enter image description", true));
            bot.global_command_create(newcommand);
        }
        });

    bot.start(dpp::st_wait);
}