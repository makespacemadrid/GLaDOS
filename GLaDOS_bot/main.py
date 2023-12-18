import moab
import time
import asyncio

print("start!")

try:
    moab.run_bots()
except KeyboardInterrupt:
    print("Bot is stopping...")
print('Successfully started (Press Ctrl+C to stop)')

while True:
    try:
        moab.process_msgs()
        time.sleep(0.33)

    except KeyboardInterrupt:
        print("Bot is stopping...")
