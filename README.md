# Queopardy

Jeopardy in Qt using QtQuick/QML

## Usage

```
./queopardy board.json
```

All paths mentioned in the board file are resolved relative to the file itself.

## Attribution

The file `duck.jpg` is licensed from the original publisher Eva Rinaldi under the terms of CC BY-SA 2.0 [1].

[1] https://www.flickr.com/photos/evarinaldiphotography/8374802487/

## Protocol

Queopardy includes a text-based WebSocket server for remote buzzers to connect to.

| Sender | Syntax | Description |
|--------|--------|-------------|
| Client | `hello <name...>` | Connect as player `name`. If the player exists and is not already controlled by another client, you will get control of it. If it doesn't exist and the game is not started yet, it is created and assigned to you. Otherwise you receive an error. |
| Server | `ok <message...>` | Something succeeded. This message is mostly useful when developing. |
| Server | `error <message...>` | Something went wrong. This message is mostly useful when developing. |
| Server | `welcome` | Response to a successful `hello` command. Followed by `category`, `tile`, and `answer` commands. |
| Server | `category <category_id> <label...>` | Send once for each category on the board, after the `welcome` message. |
| Server | `tile <category_id> <tile_id> <points> <state>` | The category with the given ID has the given label. `state` is `revealed` (question has been answered) or `hidden` (question is available for answering) |
| Server | `answer <category_id> <tile_id> <answer_id> <score> <player...>` | The category with the given ID has the given label |
| Client | `color <r> <g> <b>` | Set your player color (r, g, b are unsigned byte values). |
| Client | `nextColor` | Automatically get the next free pre-defined color |
| Server | `turn <player...>` | It's  |
| Client | `select <category_id> <tile_id>` | Automatically get the next free pre-defined color |
| Server | `question open <question...>` | A question has been selected. Everything after `open ` is the question text. Image questions only send `<image>`. |
| Server | `buzzer open` | It is now allowed to buzz. |
| Client | `buzz` | Try to buzz. |
| Server | `buzzer closed <name...>` | Someone buzzed, so you can no longer buzz yourself. |
| Server | `buzzer closed` | The buzzer was closed because the question was cancelled. |
| Server | `question closed` | The question was closed. |
| Server | `score <score> <player...>` | A player's score has changed |
| Server | `end` | The game has ended. No further messages will be accepted. |

### Example

```
# Client sends the first message
[CLIENT_TO_SERVER] hello Alice
# Server responds with "welcome" and the current state of the board
[SERVER_TO_CLIENT] welcome
[SERVER_TO_CLIENT] category 0 First Category
[SERVER_TO_CLIENT] tile 0 0 100 hidden
[SERVER_TO_CLIENT] tile 0 1 200 hidden
[SERVER_TO_CLIENT] tile 0 2 300 hidden
[SERVER_TO_CLIENT] tile 0 3 400 hidden
[SERVER_TO_CLIENT] tile 0 4 500 hidden
[SERVER_TO_CLIENT] category 1 Second Category
[SERVER_TO_CLIENT] tile 1 0 100 hidden
[SERVER_TO_CLIENT] tile 1 1 200 hidden
... (so on for further categories and tiles)
# The game begins when the server first gives someone control of the board
[SERVER_TO_CLIENT] turn Alice
# Alice now gets to select a question; she chooses category 2 tile 3
[CLIENT_TO_SERVER] select 2 3
# The server sends everyone the question, and opens the buzzer
[SERVER_TO_CLIENT] question open First Question
[SERVER_TO_CLIENT] buzzer open
# Alice buzzes
[CLIENT_TO_SERVER] buzz
# But bob was faster, so server tells everyone that Bob buzzed
[SERVER_TO_CLIENT] buzzer closed Bob
# Bob will now answer. Let's say he got it wrong; he gets -400 and the buzzer reopens
[SERVER_TO_CLIENT] answer 2 3 0 -400 Bob
[SERVER_TO_CLIENT] score -400 Bob
[SERVER_TO_CLIENT] buzzer open
# Alice buzzes
[CLIENT_TO_SERVER] buzz
# This time she got it
[SERVER_TO_CLIENT] buzzer closed Alice
# So she answers correctly
[SERVER_TO_CLIENT] answer 2 3 1 400 Alice
[SERVER_TO_CLIENT] score 400 Alice
[SERVER_TO_CLIENT] question closed
# Now it's the next player's turn
[SERVER_TO_CLIENT] turn Bob
# and so the game continues, and then ends.
[SERVER_TO_CLIENT] end
```