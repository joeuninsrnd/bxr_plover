[![GitHub version](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover.svg)](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c5b68ed507a24791a1b41f3d9c5b1dc1)](https://www.codacy.com/manual/jun08111/bxr_plover?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=joeunins/bxr_plover&amp;utm_campaign=Badge_Grade)
## GETTING STARTED
 BXR_Plover is one of the Open OS R&D project in Korean government. It will be able to detect and protect sensitive data.<br>
BXR_Plover structure is Server and Client in RabbitMQ and MariaDB. Now you can see UI that is made from GTK+3 and Glade, can detect sensitive data like Resident registration number, Driver License numver, Foreigner registration number and Passport number in txt file.<br><br>
**&#35;&#35;Process and Tools&#35;&#35;**<br>Client <=> Server <=> Web <=> Administrator<br><br>
*&#35;Client&#35;*<br>Debian 10 <br>GTK+ 3.24.10 <br>RabbitMQ 3.7.17 <br><br>
*&#35;Server&#35;*<br>Debian 10 <br>RabbitMQ 3.7.17 <br>MariaDB 10.4.7 <br><br>
*&#35;Web&#35;*<br> Tomcat <br>Web/WAS 9.0.21 <br><br>
*&#35;Administrator&#35;*<br>Web Console

## INSTALL
Install

## RUN
1. download
1. build
gcc -o bxr_plover main.c -lrabbitmq -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic</br>
on /usr/include input *.h files than you can fix bug.</br>
1. run
./bxr_plover
</br>
Install and  Run bxr_plover

## LICENSE
This project is licensed under the GPL v3.0 License.<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/LICENSE.md<br>
https://github.com/joeuninsrnd/bxr_plover/wiki/about-LICENSE

## CONTRIBUTING
https://joeunins.slack.com<br>
https://joeuninsrnd.tistory.com<br>
https://joeuninsrnd.github.io<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/CONTRIBUTING.md<br>
old: https://github.com/joeunins/bxr_plover<br>
