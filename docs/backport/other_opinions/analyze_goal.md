Look at this MongoDB repo, both the client side and the server side (specifically mongod and the access client, mongosh). Review the branches for versions 4, 5, 6, 7, and 8.

Analyze APIs to identify:
1. The commands sent from the client to the server and the APIs the server can acccept/reject/fail 
2. The different flags and commands available in each version
3. The API evolution from version 4 to 8, specifically how the API surface has evolved
4. The differences and gaps between versions, such as which commands are supported by version 8 but not by version 6.
Do this step by step, analyze each version's command parser and handler, give me a detailed breakdown of each version in a separate document, and then a final detailed comparions. Do not miss any details or small differences, make it complete and fully document each version. If necessary, document semantic difference for the flags or commands too. 

Create a implementation level very detailed analysis version by version, list all the details so that a user can follow the documents to accomplish backports without consulting the original source code. 

Assume I start with the 4.0 version codebase and I want to implement forward compatibility—essentially implementing the APIs of the later versions (v5, v6, v7, and v8).

Assumes that I only care about data model and data manipulation APIs, such as creating new types of indexes and so on. I am not interested in systems-level things such as sharding, replication, and transactions; assume those can be handled correctly and easily with the lower-level storage engines. I do care about things like security, different kinds of flags, and also newer data models and access methods.

Analyze all the API changes of the later versions to determine:
1. Which easy data APIs can be implemented on top of the 4.0 database source code easily?
2. Which versions or features are more difficult and would require a significant amount of development?
3. What are the more difficult things that need architecture changes or a significant amount of refactoring?

Write new documents on each of these features and APIs from the previous analysis. Make a very careful evaluation of each API and mark them as one of the following:
1. Very easily doable
2. Needs a lot of development
3. Very hard to do 
4. Something that should be ignored for now because it's not user-facing or it's a storage engine or systems issue.
