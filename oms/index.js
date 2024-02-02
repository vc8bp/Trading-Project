import axios from 'axios'
import env from "dotenv"
import fs from "fs"
env.config();

const apiUrl = 'https://Openapi.5paisa.com/VendorsAPI/Service1.svc/TOTPLogin';

const fetchAccessToken = async (requestToken) => {
    console.log("Fetching access token")
    const accessTokenUrl = 'https://Openapi.5paisa.com/VendorsAPI/Service1.svc/GetAccessToken';

    const accessTokenRequest = {
        head: {
            Key: process.env.UserKey
        },
        body: {
            RequestToken: requestToken,
            EncryKey: process.env.EncryptionKey,
            UserId: process.env.UserID
        }

    };

    console.log(accessTokenRequest)

    try {
        const accessTokenResponse = await axios.post(accessTokenUrl, accessTokenRequest);
        fs.writeFileSync('test.json', JSON.stringify(accessTokenResponse.data.body));
    } catch (error) {
        console.log(error)
        console.error('Error fetching access token:', error.message);
    }
};


const sendRequest = async () => {
    const requestData = {
        head: {
            Key: process.env.UserKey,
        },
        body: {
            Email_ID: process.env.number,
            TOTP: '',
            PIN: process.env.pin,
        },
    };


    let totp;
    if(process.argv.length < 3){
        return console.error("Please provide TOTP")
    } else {
        totp = process.argv[2]
    }
    if (totp.length < 6 || isNaN(totp)){
        return console.error("Please provide a valid TOTP")
    }

    requestData.body.TOTP = totp

    try {
        const response = await axios.post(apiUrl, requestData);
        const requestToken = response.data.body.RequestToken;

        await fetchAccessToken(requestToken);

    } catch (error) {
        console.log(error);
    }
};

sendRequest();
