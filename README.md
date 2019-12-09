[![GitHub version](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover.svg)](https://badge.fury.io/gh/joeuninsrnd%2Fbxr_plover)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c5b68ed507a24791a1b41f3d9c5b1dc1)](https://www.codacy.com/manual/jun08111/bxr_plover?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=joeunins/bxr_plover&amp;utm_campaign=Badge_Grade)
[![codecov](https://codecov.io/gh/jun08111/bxr_plover/branch/master/graph/badge.svg)](https://codecov.io/gh/jun08111/bxr_plover)
[![Build Status](https://travis-ci.org/jun08111/bxr_plover.svg?branch=master)](https://travis-ci.org/jun08111/bxr_plover)
<br>
![plover_logo_horizontal](https://user-images.githubusercontent.com/28081461/68651563-feef7680-056a-11ea-9102-131561bedb90.png)
<br>
## 개요
 bxr_plover(민감정보 생명주기 관리프로그램)는 최근 정부에서 진행되는 개방형 OS 도입 및 확산 프로젝트의 한 부분으로 진행되는 연구 개발로써 개방형 OS 환경에서의 민감정보 검출 및 유출을 방지하기 위한 방향으로 진행할 것이며, RabbitMQ를 사용하여 Server와 Client 통신을 하고 MariaDB, Tomcat을 사용할 예정이다. 현재 GTK와 Glade로 개발한 UI를 통해 Server/Client 구조로 txt, pdf등의 파일 형태의 민감정보(주민등록번호, 외국인등록번호, 운전면허번호, 여권번호) 검출이 가능하다.<br><br>
**&#35;&#35;프로세스 구상도 및 사용 도구&#35;&#35;**<br>Client <=> Server <=> Web <=> Administrator<br><br>
*&#35;Client&#35;*<br>Debian 10 <br>GTK+ 3.24.10 <br>RabbitMQ 3.7.17 <br><br>
*&#35;Server&#35;*<br>Debian 10 <br>RabbitMQ 3.7.17 <br>MariaDB 10.4.7 <br><br>
*&#35;Web&#35;*<br> Tomcat <br>Web/WAS 9.0.21 <br><br>
*&#35;Administrator&#35;*<br>Web Console

## 사용방법
설치후 실행

## 개발방법
1. download
1. build
$ cd bxr_plover/client/src 위치로 이동<br>
$ make<br>
$ ./plover_client<br>
1. run
./plover_client
</br>
다운로드후 plover_client실행

## 라이선스
This project is licensed under the GPL v3.0 License.<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/LICENSE.md<br>
https://github.com/joeuninsrnd/bxr_plover/wiki/about-LICENSE

## 참여방법
https://joeunins.slack.com<br>
https://joeuninsrnd.tistory.com<br>
https://joeuninsrnd.github.io<br>
https://github.com/joeuninsrnd/bxr_plover/blob/master/CONTRIBUTING.md<br>
old: https://github.com/joeunins/bxr_plover<br>
