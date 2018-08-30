const path = require('path');
const outputDir = path.join(__dirname, "./mifkad-assets/js");

const isProd = process.env.NODE_ENV === 'production';

module.exports = {
  entry: './src/frontend/Index.bs.js',
  mode: isProd ? 'production' : 'development',
  output: {
    path: outputDir,
    publicPath: outputDir,
    filename: 'Index.js',
  },
};
