Correct
	WHEN: The player who buzzed gave the correct answer
	RESULT: The player gets the points, the game returns to the board, the question is marked "revealed"

Incorrect:
	WHEN: The player who buzzed gave an incorrect answer
	RESULT: The player gets negative points, and the buzzer is opened again so someone else can try

Fail:
	WHEN: The question has been ruined somehow, but nobody specifically is at fault. E.g. the host accidentally gave too much info, an audience member yelled the answer, or nobody is willing/able to answer
	RESULT: Nobody gets any point, the game returns to the board, and the question is marked "revealed" and gets an answer by "Nobody".

Unbuzz:
	WHEN: The buzzer was accidentally triggered.
	RESULT: the buzzer is opened again and anyone can buzz.

Unclick:
	WHEN: The wrong question was clicked.
	RESULT: Nobody gets any points, the game returns to the board, and the question is marked "revealed" (but unlike "Fail", it is not marked).

NOTE: revealed questions of any kind can technically be opened again by ctrl+clicking them. Use at the host's discretion.
