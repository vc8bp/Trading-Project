import axios from 'axios'
import env from "dotenv"
env.config();

const apiUrl = 'https://Openapi.5paisa.com/VendorsAPI/Service1.svc/TOTPLogin';


const redirectToOauthLogin = (requestToken) => {
    const vendorKey = 'YOUR_VENDOR_KEY';
    const redirectURL = 'YOUR_REDIRECT_URL';
    const state = 'YOUR_STATE_VALUE'; 

    const oauthLoginURL = `https://dev-openapi.5paisa.com/WebVendorLogin/VLogin/Index?VendorKey=${vendorKey}&ResponseURL=${redirectURL}&State=${state}&RequestToken=${requestToken}`;

    console.log(`Please open the following URL in your browser to complete OAuth login:\n${oauthLoginURL}`);
};



const fetchAccessToken = async (requestToken) => {
    const accessTokenUrl = 'https://Openapi.5paisa.com/VendorsAPI/Service1.svc/GetAccessToken';

    const accessTokenRequest = {
        key: process.env.UserKey, 
        RequestToken: requestToken,
    };

    try {
        const accessTokenResponse = await axios.post(accessTokenUrl, accessTokenRequest);

        console.log('Access Token:', accessTokenResponse.data);
    } catch (error) {
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

        fetchAccessToken(response.data.RequestToken)

        console.log(response.data);
    } catch (error) {
        console.error('Error:', error.message);
    }
};

sendRequest();
