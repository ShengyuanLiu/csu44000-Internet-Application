const express = require("express")
const path = require("path")
const app = express()
const AWS = require("aws-sdk");
const AWS_ACCESS_KEY=process.env.AWS_ACCESS_KEY
const AWS_SECRET_KEY=process.env.AWS_SECRET_KEY
const PORT = process.env.PORT



AWS.config.update({
    region: 'us-east-1',
    accessKeyId: AWS_ACCESS_KEY,
    secretAccessKey: AWS_SECRET_KEY
});

var dynamodb = new AWS.DynamoDB();
var theClient = new AWS.DynamoDB.DocumentClient();

var s3 = new AWS.S3();

var s3params = {
    Bucket: 'csu44000assign2useast20',
    Key: 'moviedata.json'
};

app.use(express.static('public'));
app.get("/", function (req, res) {
    res.sendFile(path.join(__dirname + "/index.html"))
});


app.listen(PORT, function () {
    console.log("Movie App is listening on " +PORT)
});

app.post('/create', (req, res) => {
    console.log("Creating")
    var params = {
        TableName: "Movies",
        KeySchema: [
            { AttributeName: "year", KeyType: "HASH" },  //Partition key
            { AttributeName: "title", KeyType: "RANGE" }  //Sort key
        ],
        AttributeDefinitions: [
            { AttributeName: "year", AttributeType: "N" },
            { AttributeName: "title", AttributeType: "S" }
        ],
        ProvisionedThroughput: {
            ReadCapacityUnits: 10,
            WriteCapacityUnits: 10
        },
        BillingMode: "PAY_PER_REQUEST"
    };
    dynamodb.createTable(params, function (err, data) {
        if (err) {
            console.error("Unable to create table. Error JSON:", JSON.stringify(err, null, 2));
        } else {
            console.log("Created table success. Table description JSON:", JSON.stringify(data, null, 2));
        }
    });

    s3.getObject(s3params, function (err, data) {
        if (err) {
            console.log(err, err.stack);
        } else {
            var allMovies = JSON.parse(data.Body.toString());
             allMovies.forEach(function (movie) {
                var params = {
                    TableName: "Movies",
                    Item: {
                        "year": movie.year,
                        "title": movie.title,
                        "rating": movie.info.rating,
                        "rank": movie.info.rank,
                        "release": movie.info.release_date
                    }
                };
                

                theClient.put(params, function (err, data) {
                    if (err) {
                        console.error("Unable to add movie", movie.title, ". Error JSON:", JSON.stringify(err, null, 2));
                    } else {
                        console.log("Add the movie success! :", movie.title);
                    }
                });
            });
        }
        console.log("Database created successfully");
    })
});


app.post('/query/:title/:year', (req, res) => {
    console.log("Query time")
    var myArray = {
        datalist :[]
    }
    var year = parseInt(req.params.year)
    var title = req.params.title
    var params = {
        TableName : "Movies",
        ProjectionExpression:"#yr, title, rating, #r, #re",
        KeyConditionExpression: "#yr = :yyyy and begins_with (title, :letter1)",
        ExpressionAttributeNames:{
            "#yr": "year",
            "#r":"rank",
            "#re":"release"
        },
        ExpressionAttributeValues: {
            ":yyyy": year,
            ":letter1": title
        }
    };

    theClient.query(params, function(err, data) {
        if (err) {
            console.log("Unable to query. Error:", JSON.stringify(err, null, 2));
        } else {
            console.log("Query succeeded.");
            data.Items.forEach(function(item) {
                console.log(item.year +' '+ item.title+'' + item.rating);
                var inputYear = item.year
                var inputTitle = item.title
                var inputRate = item.rating
                var movieRank = item.rank
                var releaseDate = item.release
                myArray.datalist.push(
                    {
                        Title: inputTitle,
                        Year : inputYear,
                        Rating: inputRate,
                        Rank: movieRank,
                        Release: releaseDate
                    }
                )
            });
            console.log('Done Printing')
            res.json(myArray)
        }
    });
});



app.post('/destroy', (req, res) => {
    console.log("Destroying... Please be patient.");
    var params = {
        TableName : "Movies",
    };
    dynamodb.deleteTable(params, function(err, data) {
        if (err) {
            console.error("Unable to delete table. Error JSON:", JSON.stringify(err, null, 2));
        } else {
            console.log("Deleted table success. Table description JSON:", JSON.stringify(data, null, 2));
        }
    });
});



