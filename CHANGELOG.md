Implemented both TCP and UDP variant.
/auth functional
/join functional
/rename functional
/help functional
sending messages functional
receiving messages functional

Tested only on the reference server.

Assumes messages incoming messages not adhering to the protocol grammar are malformed.
Assumes all messages starting with '/' are commands and not meant for the server.
Assumes errors sent to the server are also considered client errors and are printed on STDOUT in the local error format.

In case of half-written input on STDIN, Ctrl+D only exits after 2 presses.
Handling of messages that exceed the protocol allowed limit is NOT implemented.

Generative AI and LLMs were only used for discussing problem approaches, such as the decision to implement blocking system calls instead of threads, and the explanation of certain C++ principles.
The only exceptions are some functions located in 'string_functions.cpp' (namely extract_value and tokenize) and some functions in 'udp_message.cpp' (namely push_two_bytes, pop_two_bytes, push_string, pop_string)
