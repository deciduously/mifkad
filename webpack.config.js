const path = require('path');
const outputDir = path.join(__dirname, "webapp/build/");

const isProd = process.env.NODE_ENV === 'production';

module.exports = {
  entry: './webapp/src/Index.bs.js',
  mode: isProd ? 'production' : 'development',
  output: {
    path: outputDir,
    publicPath: outputDir,
    filename: 'Index.js',
  },
};
