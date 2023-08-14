import moab


print("start!")


try:
    print('Successfully started (Press Ctrl+C to stop)')
    moab.run_bots()
except KeyboardInterrupt:
    print("Bot is stopping...")

