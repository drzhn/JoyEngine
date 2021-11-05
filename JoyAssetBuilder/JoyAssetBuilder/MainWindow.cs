using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace JoyAssetBuilder
{
    public partial class MainWindow : Form
    {
        AssetPanelViewController panelViewController;
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            panelViewController = new AssetPanelViewController(assetTreeView);
        }

        private void expandAll_Click(object sender, EventArgs e)
        {
            panelViewController.ExpandAll();
        }

        private void collapseAll_Click(object sender, EventArgs e)
        {
            panelViewController.CollapseAll();
        }
    }
}
