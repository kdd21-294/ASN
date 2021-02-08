This is the code and data for paper(ID 249) "Acquaintance Social Network: A Description of People’s Communication in the Physical World" in KDD2021.

1. Dataset
We use 3 real-world datsets. 
The first one is email-Eu-core temporal network, the download link is http://snap.stanford.edu/data/email-Eu-core-temporal.html.
The second one is weibo dataset, which is collected from https://weibo.cn/
The laset one is Stack Overflow temporal network, the link is http://snap.stanford.edu/data/sx-stackoverflow.html.

Data format.
The first line：Total_nodes  Time_span
Total_nodes, means the total number of vertices in the graph
Time_span, means the total number of time intervals
The other lines: SRC TGT UNIXTS 
SRC, means the id of one vertex (user) of an edge
TGT, means the id of the other vertex of the edge
UNIXTS, means the time interval of the latest interaction

2. C++ Code (main.cpp)
The experiments over Weibo and StackOverflow are implemented by C++.
In Line 25, you can set the parameters to identify ASN or TSN.
You can modify the code of Line 487 to read different datasets.

3. Spark Code (SocialNetworkAnalyze)
The experiments over email-Eu-core temporal network are implemented by Scala on Spark.
The dependencies are in pom.xml
You can import the project into an IDE such as Intelij Idea to run the code.
