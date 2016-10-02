/*
 *  Abuse — dark 2D side-scrolling platform game
 *  Copyright © 1995 Crack dot Com
 *  Copyright © 2005—2016 Sam Hocevar <sam@hocevar.net>
 *
 *  This software was released into the Public Domain. As with most public
 *  domain software, no warranty is made or implied by Crack dot Com, by
 *  Jonathan Clark, or by Sam Hocevar.
 */

#pragma once

#include <unistd.h>
#include "sock.h"
#include "ghandler.h"

class game_client : public game_handler
{
  net_socket *client_sock;             // connection to server as a client
  int wait_local_input;
  int process_server_command();
  net_address *server_data_port;
  public :

  game_client(net_socket *client_sock, net_address *server_addr);
  int process_net();
  int input_missing();
  void add_engine_input();
  virtual int start_reload();
  virtual int end_reload(int disconnect=0);
  virtual int kill_slackers();
  virtual int quit();
  virtual ~game_client();
};

