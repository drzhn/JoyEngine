using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace JoyAssetBuilder
{

    interface IBuildable
    {
        void Build();
        bool IsBuilding { get;}
        bool Builded { get; }
    }
    public enum AssetType
    {
        Folder,
        Model,
        Texture,
        Shader
    }
    public class AssetTreeNode : TreeNode, IBuildable
    {
        AssetType m_type;
        string m_path;

        bool IBuildable.IsBuilding => throw new NotImplementedException();

        bool IBuildable.Builded => throw new NotImplementedException();

        public AssetTreeNode(AssetType type, string path)
        {
            m_type = type;
            m_path = path;
            Text = Path.GetFileName(path);
            SetImage();
            SetColor();
        }

        private void SetImage()
        {
            ImageKey = m_type.ToString();
            SelectedImageKey = m_type.ToString();
        }

        private void SetColor()
        {
            if (File.Exists(m_path + ".data"))
            {
                BackColor = System.Drawing.Color.DarkSeaGreen;
            }
            else
            {
                BackColor = System.Drawing.Color.IndianRed; 
            }
        }

        void IBuildable.Build()
        {
            throw new NotImplementedException();
        }
    }
}
