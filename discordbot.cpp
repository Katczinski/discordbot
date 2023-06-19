// discordbot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define CURL_STATICLIB

#include <iostream>
#include <dpp/dpp.h>
#include "openai.hpp"

const std::string    BOT_TOKEN = "MTExNTU3MDc3MzIxODgyNDI5Mg.GNzQdn.TOG5YcenQFJm3fb64LIzjI_yl68R6dfRXedLCA";
std::string model = "gpt-3.5-turbo";

int main() {
    dpp::cluster bot(BOT_TOKEN);
    bot.intents |= dpp::i_message_content;

    openai::start();
    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&bot](const dpp::message_create_t& event) {
    	if (event.msg.author == bot.me)
			return;
		bool is_mentioned = false;
		for (auto mention : event.msg.mentions) {
            if (mention.first == bot.me)
				is_mentioned = true; 
        }
		if (event.msg.is_dm())
			is_mentioned = true;
		if (is_mentioned) {
			std::string msg = event.msg.content;
            nlohmann::json array;
            array.push_back(nlohmann::json::object({ { "role", "user" }, { "content", msg } }));
            nlohmann::json request {   
										{"model", model },
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
        });
    bot.on_slashcommand([&](const dpp::slashcommand_t& event) {
        if (event.command.get_command_name() == "image") {
            std::string description = std::get<std::string>(event.get_parameter("description"));
			
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
		// else if (event.command.get_command_name() == "model") {
		// 	nlohmann::json array;
		// 	std::string new_model = std::get<std::string>(event.get_parameter("model"));
		// 	array.push_back(nlohmann::json::object({ { "role", "user" }, { "content", "Tell me the language model you are based on" } }));
		// 	nlohmann::json request {    {"model", new_model },
		// 								{ "messages", array },
		// 								{ "temperature", 0.2 },
		// 	};
		// 	std::string response;
		// 	event.thinking();
		// 	try {
		// 		auto chat = openai::chat().create(request);
		// 		nlohmann::json json = nlohmann::json::parse(chat.dump(2));
		// 		response = json.at("choices").at(0).at("message").at("content");
		// 		model = new_model;
		// 	}
		// 	catch (const std::exception& e) {
		// 		nlohmann::json json = nlohmann::json::parse(e.what());
		// 		response = json.at("message");
		// 	}
		// 	event.edit_original_response(response);
		// 	return;
		// }
        });
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            /* Create a new global command on ready event */
            dpp::slashcommand image_command("image", "Generate image", bot.me.id);
            image_command.add_option(dpp::command_option(dpp::co_string, "description", "Enter image description", true));
            bot.global_command_create(image_command);
			// dpp::slashcommand model_command("model", "Change model", bot.me.id);
			// model_command.add_option(dpp::command_option(dpp::co_string, "model", "Enter model id", true));
			// bot.global_command_create(model_command);
        }
        });

    bot.start(dpp::st_wait);
}
