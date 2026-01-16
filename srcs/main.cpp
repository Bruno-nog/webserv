/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdavi-al <sdavi-al@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 14:31:56 by brunogue          #+#    #+#             */
/*   Updated: 2026/01/16 19:05:17 by sdavi-al         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>

void runServer();
int main() {
    try
	{
        runServer();
    } catch (std::exception &e)
	{
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}