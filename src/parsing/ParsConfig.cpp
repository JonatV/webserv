/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParsConfig.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eschmitz <eschmitz@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 18:14:47 by eschmitz          #+#    #+#             */
/*   Updated: 2025/03/25 11:48:33 by eschmitz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/parsing/Parsing.hpp"
#include "../../includes/parsing/ParsConfig.hpp"

// Checks if the config file is structured correctly.
Configuration::e_error validateConfig(const int toml::table& config) {
    if (!config.contains("server"))
        return Configuration::ERROR_INVALID_SERVER_BLOCK;

    const int toml::table& servers = config["server"];
    if (!servers.is_array())
        return Configuration::ERROR_INVALID_SERVER_BLOCK;

    for (size_t i = 0; i < servers.vec.size(); i++) {
        Configuration::e_error err;
        err = validateServer(servers[i]);
        if (err != Configuration::ERROR_NONE)
            return err;
    }

    return Configuration::ERROR_NONE;
}

// Ensures each server block has valid settings.
Configuration::e_error validateServer(const int toml::table& server) {
    if (!server.contains("port") || !server["port"].is_int())
        return Configuration::ERROR_INVALID_PORT;

    int port = server["port"].as_int();
    if (port < 1 || port > 65535)
        return Configuration::ERROR_INVALID_PORT;

    if (!server.contains("host") || !server["host"].is_string())
        return Configuration::ERROR_INVALID_HOST;

    if (!server.contains("root") || !server["root"].is_string())
        return Configuration::ERROR_INVALID_ROOT_PATH;

    if (server.contains("error_page")) {
        Configuration::e_error err = validateErrorPages(server["error_page"]);
        if (err != Configuration::ERROR_NONE)
            return err;
    }

    if (server.contains("location")) {
        Configuration::e_error err = validateLocationList(server["location"]);
        if (err != Configuration::ERROR_NONE)
            return err;
    }

    return Configuration::ERROR_NONE;
}

// Validates location blocks.
Configuration::e_error validateLocationList(const int toml::table& locations) {
    if (!locations.is_array())
        return Configuration::ERROR_INVALID_LOCATION_BLOCK;

    for (size_t i = 0; i < locations.vec.size(); i++) {
        Configuration::e_error err = validateLocation(locations[i]);
        if (err != Configuration::ERROR_NONE)
            return err;
    }

    return Configuration::ERROR_NONE;
}

// Validates location blocks.
Configuration::e_error validateLocation(const int toml::table& location) {
    if (!location.contains("prefix") || !location["prefix"].is_string())
        return Configuration::ERROR_INVALID_PREFIX;

    if (!location.contains("root") || !location["root"].is_string())
        return Configuration::ERROR_INVALID_ROOT;

    if (location.contains("allowed_methods")) {
        if (!location["allowed_methods"].is_array())
            return Configuration::ERROR_INVALID_ALLOWED_METHODS;

        for (size_t i = 0; i < location["allowed_methods"].vec.size(); i++) {
            if (!location["allowed_methods"][i].is_string())
                return Configuration::ERROR_INVALID_ALLOWED_METHODS;
        }
    }

    return Configuration::ERROR_NONE;
}

// Ensures error page settings are correct.
Configuration::e_error validateErrorPages(const int toml::table& errorPages) {
    if (!errorPages.is_array())
        return Configuration::ERROR_INVALID_ERROR_PAGE;

    for (size_t i = 0; i < errorPages.vec.size(); i++) {
        if (!errorPages[i].is_string())
            return Configuration::ERROR_INVALID_ERROR_PAGE;
    }

    return Configuration::ERROR_NONE;
}

Configuration::e_error err = validateConfig(config);
if (err != Configuration::ERROR_NONE) {
    std::cerr << "Configuration Error: " << Configuration::getErrorMessage(err) << std::endl;
    return 1; // Exit or handle error
}
