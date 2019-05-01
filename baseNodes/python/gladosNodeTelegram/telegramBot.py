#!/usr/bin/env python
# -*- coding: utf-8 -*-


import logging
import time

from telegram import InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import Updater, CommandHandler, CallbackQueryHandler, MessageHandler, Filters
from functools import wraps
from gladosMQTT import debug

botToken		= ''
helpText		= "Your use example here..."
botMasterUser 	= 0
userList		= []
adminList		= []
updater     	= None
bot				= None
dp 				= None

# Enable logging
logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)


def tellTheAdmins(msg,context) :
	debug(msg)
	if botMasterUser != 0 :
		bot.send_message(chat_id=botMasterUser,text=msg)
	for admin in adminList :
		bot.send_message(chat_id=admin,text=msg)

def isUser(user,context) :
	if botMasterUser == 0 and len(userList) == 0 and len(adminList) == 0 : 
		debug("No users, public mode!")
		return True

	if(user == botMasterUser) : return True	
	if user in adminList 	  : return True
	
	if user not in userList :
		tellTheAdmins("El usuario " + str(user) + " está intentando acceder al bot",context)
		return False
	return True

def isAdmin(user,context) :
	if(user == botMasterUser) : return True
	if user not in adminList :
		tellTheAdmins("El usuario " + str(user) + ' está intentando acceder al ADMINISTRADOR del bot',context)
		return False
	return True

def addUser(user) :
	userList += user

def rmUser(user) :
	pass

def addAdmin(user) :
	adminList += user

def rmAdmin(user) :
	pass


def start(update, context):
	if not isUser(update.message.chat_id,context) :
		return
	update.message.reply_text('La resistencia es futil, preparate para ser asimilado')
	

def help(update, context):
	if not isUser(update.message.chat_id,context) :
		return
	update.message.reply_text(helpText)

def processMSG(update, context):
	if not isUser(update.message.chat_id,context) :
		return
	update.message.reply_text('echo:'+update.message.text)
	debug("[TGBot] rcv, id: " + str(update.message.chat_id) + "  msg: " + update.message.text)

def error(update, context):
	logger.warning('Update "%s" caused error "%s"', update, context.error)


def initBot(token , masterUser = 0 , users = [] , admins = [] , helpT = 'Your help here...') :
	global botToken
	global userList
	global adminList
	global botMasterUser
	global updater
	global dp
	global bot
	global helpText
	
	botMasterUser 	= masterUser
	userList  		= users
	adminList 		= admins
	botToken  		= token
	helpText  		= helpT
	
	updater = Updater(botToken, use_context=True)
	bot = updater.bot
	debug("[TGBot] Init ,master user: " + str(botMasterUser))
	debug(bot.get_me())
	# Get the dispatcher to register handlers
	dp = updater.dispatcher

	# on different commands - answer in Telegram
	dp.add_handler(CommandHandler("start", start))
	dp.add_handler(CommandHandler("help" , help))
	# on noncommand i.e message - echo the message on Telegram
	dp.add_handler(MessageHandler(Filters.text, processMSG))

	# log all errors
	dp.add_error_handler(error)

	# Start the Bot
	updater.start_polling()
