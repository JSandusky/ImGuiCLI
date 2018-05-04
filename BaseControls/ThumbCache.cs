using Microsoft.Xna.Framework.Graphics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ImGuiControls
{
    public class ThumbCache
    {
        internal class ThumbRecord
        {
            internal Texture2D texture_;
            internal int counter_;
        }

        Dictionary<string, ThumbRecord> thumbnails_ = new Dictionary<string, ThumbRecord>();
        GraphicsDevice device_;

        public ThumbCache(GraphicsDevice device)
        {
            device_ = device;
        }

        /// TODO: thread this, actually kind of tricky because shell API won't survive just having tasks tossed at it.
        public Texture2D GetOrCreateThumbnail(string path)
        {
            if (thumbnails_.ContainsKey(path))
            {
                var rec = thumbnails_[path];
                rec.counter_++;
                return rec.texture_;
            }

            System.Drawing.Bitmap bmp = null;
            if (System.IO.File.Exists(path))
                bmp = Microsoft.WindowsAPICodePack.Shell.ShellFile.FromFilePath(path).Thumbnail.Bitmap;
            else if (System.IO.Directory.Exists(path))
                bmp = Microsoft.WindowsAPICodePack.Shell.ShellFileSystemFolder.FromFolderPath(path).Thumbnail.Bitmap;
            if (bmp != null)
            {
                Texture2D tex = BitmapToTexture2D(bmp, device_);
                thumbnails_[path] = new ThumbRecord { texture_ = tex, counter_ = 0 };
                return tex;
            }
            return null;
        }

        public void Clear(int countThreshold)
        {
            if (countThreshold == 0)
            {
                ThumbRecord[] thumbs = thumbnails_.Values.ToArray();
                foreach (var t in thumbs)
                    t.texture_.Dispose();
                thumbnails_.Clear();
            }
            else
            {
                ThumbRecord[] thumbs = thumbnails_.Values.ToArray();
            }
        }

        // Source: http://xboxforums.create.msdn.com/forums/t/3051.aspx
        public static Texture2D BitmapToTexture2D(System.Drawing.Bitmap image, GraphicsDevice GraphicsDevice)
        {
            if (GraphicsDevice == null)
                return null;
            // Buffer size is size of color array multiplied by 4 because   
            // each pixel has four color bytes  
            int bufferSize = image.Height * image.Width * 4;

            // Create new memory stream and save image to stream so   
            // we don't have to save and read file  
            System.IO.MemoryStream memoryStream = new System.IO.MemoryStream(bufferSize);
            image.Save(memoryStream, System.Drawing.Imaging.ImageFormat.Bmp);
            memoryStream.Seek(0, System.IO.SeekOrigin.Begin);

            // Creates a texture from IO.Stream - our memory stream  
            Texture2D texture = Texture2D.FromStream(GraphicsDevice, memoryStream);
            return texture;
        }
    }
}
