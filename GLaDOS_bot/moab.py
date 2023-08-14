import os
import threading

import GladosIA
import GladosTelegramClient
import GladosDiscordClient
import GladosSlackClient

def run_bots() :
#  GladosDiscordClient.run_bot(GladosIA.askGLaDOS)
#  GladosSlackClient.run_bot(GladosIA.askGLaDOS)
  tg_t = threading.Thread(target=	GladosTelegramClient.run_bot, args=(GladosIA.askGLaDOS,))
  dc_t = threading.Thread(target=	GladosDiscordClient.run_bot , args=(GladosIA.askGLaDOS,))
  sl_t = threading.Thread(target=	GladosSlackClient.run_bot   , args=(GladosIA.askGLaDOS,))

  tg_t.start()
  dc_t.start()
  sl_t.start() 

  tg_t.join()
#  dc_t.join()
