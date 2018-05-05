using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using ImGuiCLI;
using Microsoft.Xna.Framework.Graphics;

namespace ImGuiControls
{
    public class FileBrowser
    {
        internal class DirEntry
        {
            internal string directory_;
            internal string shortName_;
            internal List<DirEntry> children_ = new List<DirEntry>();

            internal void PopulateChildren()
            {
                string[] dirs = System.IO.Directory.GetDirectories(directory_);
                if (dirs != null)
                {
                    for (int i = 0; i < dirs.Length; ++i)
                    {
                        DirEntry ent = new DirEntry { directory_ = dirs[i], shortName_ = new System.IO.DirectoryInfo(dirs[i]).Name };
                        ent.PopulateChildren();
                        children_.Add(ent);
                    }
                }
            }
        }

        public string RootPath { get; set; }

        string newDirectory_;
        public string CurrentDirectory { get; set; }

        List<string> currentItems_ = new List<string>();
        DirEntry rootEntry_;

        bool asDetail_ = false;
        public bool ShowAsDetail { get { return asDetail_; } set { asDetail_ = value; } }

        public List<string> Favorites { get; private set; } = new List<string>();
        public List<string> Recents { get; private set; } = new List<string>();

        ThumbCache thumbCache_;
        public FileBrowser(GraphicsDevice deviceForCache)
        {
            thumbCache_ = new ThumbCache(deviceForCache);
            RootPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
            FillDirTree();
            CurrentDirectory = RootPath;
            UpdateItems();
        }

        public FileBrowser(string rootPath)
        {
            RootPath = rootPath;
            FillDirTree();
            CurrentDirectory = RootPath;
            UpdateItems();
        }

        /// <summary>
        /// Override (or implement) for specialized filtering in the list view
        /// </summary>
        public virtual bool ExcludeFilesystemItem(string path)
        {
            return false;
        }

        public void DrawAsWindow(string title)
        {
            if (ImGuiCli.Begin(title, ImGuiWindowFlags_.MenuBar | ImGuiWindowFlags_.ResizeFromAnySide))
            {
                DrawMenuBar();
                Draw();
            }
            ImGuiCli.End();
        }

        public void DrawAsDock(string title)
        {
            if (ImGuiDock.BeginDock(title, ImGuiWindowFlags_.MenuBar | ImGuiWindowFlags_.ResizeFromAnySide))
            {
                DrawMenuBar();
                Draw();
            }
            ImGuiDock.EndDock();
        }

        ImGuiCLI.ImGuiTextFilter filter_ = new ImGuiTextFilter();
        public void DrawMenuBar()
        {
            ImGuiCli.BeginMenuBar();
            if (!CurrentDirectory.EndsWith(":") && !CurrentDirectory.EndsWith(":\\"))
            {
                //ImGuiEx.PushBoldFont();
                if (ImGuiCli.Button(ICON_FA.LEVEL_UP_ALT, new Vector2(32, 0)))
                {
                    CurrentDirectory = System.IO.Directory.GetParent(CurrentDirectory).FullName;
                    UpdateItems();
                }
                if (ImGuiCli.IsItemHovered()) ImGuiCli.SetTooltip("To parent directory");
                //ImGuiEx.PopFont();
            }
            ImGuiCli.Text(CurrentDirectory);
            ImGuiCli.PushItemWidth(ImGuiCli.GetContentRegionAvailWidth() * 0.7f - 70);
            filter_.Draw(ICON_FA.FILTER + " Filter");
            ImGuiCli.PopItemWidth();

            //ImGuiEx.PushBoldFont();
            ImGuiCli.SetCursorPosX(ImGuiCli.GetWindowWidth() - ImGuiCli.CalcTextSize(ICON_FA.LIST_UL + "  +  " + ICON_FA.STAR).X); //70);
            if (ImGuiCli.Button(ICON_FA.LIST_UL))
                asDetail_ = !asDetail_;
            if (ImGuiCli.IsItemHovered())
                ImGuiCli.SetTooltip("Toggle details view");

            if (ImGuiCli.Button("+" + ICON_FA.STAR))
            {
                if (!Favorites.Contains(CurrentDirectory))
                    Favorites.Add(CurrentDirectory);
            }
            if (ImGuiCli.IsItemHovered())
                ImGuiCli.SetTooltip("Add to favorites");
            //ImGuiEx.PopFont();

            ImGuiCli.EndMenuBar();
        }

        public void Draw()
        {

            if (newDirectory_ != null && newDirectory_ != CurrentDirectory)
            {
                CurrentDirectory = newDirectory_;
                UpdateItems();
            }

        // Left hand side, draw the local roots, favorites and recent
            float totalWidth = ImGuiCli.GetContentRegionAvailWidth();
            float w = totalWidth;
            float fontHeight = ImGuiCli.GetFontSize();
            float fSize = fontHeight * 16;
            w = Math.Min(w * 0.33f, fSize);

            float yAvail = ImGuiCli.GetContentRegionAvail().Y;

            ImGuiCli.BeginChild("##folder_tree", new Vector2(w, yAvail), true);
            //ImGuiEx.PushBoldFont();
            bool localOpen = ImGuiCli.CollapsingHeader(ICON_FA.FOLDER + " LOCAL");
            //ImGuiEx.PopFont();
            if (localOpen)
                DrawTree(rootEntry_);

            //ImGuiEx.PushBoldFont();
            bool favoritesOpen = ImGuiCli.CollapsingHeader(ICON_FA.STAR + " FAVORITES");
            //ImGuiEx.PopFont();
            if (favoritesOpen)
            {
                ImGuiCli.Indent();
                for (int i = 0; i < Favorites.Count; ++i)
                {
                    ImGuiCli.PushID(i + 1);

                    Texture2D thumbnail = thumbCache_.GetOrCreateThumbnail(Favorites[i]);
                    if (thumbnail != null)
                    {
                        ImGuiCli.Image(thumbnail, new Vector2(fontHeight, fontHeight));
                        ImGuiCli.SameLine();
                    }
                    string dispValue = Favorites[i].Substring(Favorites[i].Substring(0, Favorites[i].LastIndexOf("\\")).LastIndexOf("\\") + 1);
                    ImGuiCli.Text(dispValue);
                    if (ImGuiCli.IsItemClicked(0))
                    {
                        CurrentDirectory = Favorites[i];
                        UpdateItems();
                    }
                    else if (ImGuiCli.IsItemClicked(1))
                        ImGuiCli.OpenPopup("#favorite_ctx");
                    if (ImGuiCli.BeginPopup("#favorite_ctx", ImGuiWindowFlags_.None))
                    {
                        if (ImGuiCli.MenuItem(ICON_FA.MINUS + " Remove"))
                        {
                            Favorites.RemoveAt(i);
                            --i;
                        }
                        ImGuiCli.EndPopup();
                    }
                    ImGuiCli.PopID();
                }
                ImGuiCli.Unindent();
            }

            //ImGuiEx.PushBoldFont();
            bool recentOpen = ImGuiCli.CollapsingHeader(ICON_FA.CLOCK + " RECENT");
            //ImGuiEx.PopFont();
            if (recentOpen)
            {
                ImGuiCli.Indent();
                for (int i = 0; i < Recents.Count; ++i)
                {
                    ImGuiCli.PushID(i + 1);

                    Texture2D thumbnail = thumbCache_.GetOrCreateThumbnail(Recents[i]);
                    if (thumbnail != null)
                    {
                        ImGuiCli.Image(thumbnail, new Vector2(fontHeight, fontHeight));
                        ImGuiCli.SameLine();
                    }
                    string dispValue = Recents[i].Substring(Recents[i].Substring(0, Recents[i].LastIndexOf("\\")).LastIndexOf("\\") + 1);
                    ImGuiCli.Text(dispValue);
                    if (ImGuiCli.IsItemClicked(0))
                    {
                        CurrentDirectory = Recents[i];
                        UpdateItems();
                    }
                    else if (ImGuiCli.IsItemClicked(1))
                        ImGuiCli.OpenPopup("#Recents_ctx");
                    if (ImGuiCli.BeginPopup("#Recents_ctx", ImGuiWindowFlags_.None))
                    {
                        if (ImGuiCli.MenuItem(ICON_FA.MINUS + " Remove"))
                        {
                            Recents.RemoveAt(i);
                            --i;
                        }
                        ImGuiCli.EndPopup();
                    }
                    ImGuiCli.PopID();
                }
                ImGuiCli.Unindent();
            }

            ImGuiCli.EndChild();

            ImGuiCli.SameLine();

        // Right hand side, list the current items
            float contentWidth = totalWidth - w - ImGuiStyle.WindowPadding.X;
            ImGuiCli.BeginChild("##dir_content_region", new Vector2(contentWidth, yAvail), true);

            float itemSize = asDetail_ ? 48 : 128;

            int columns = (int)(contentWidth / (asDetail_ ? 256.0f : 128.0f));
            columns = columns < 1 ? 1 : columns;

            ImGuiCli.Columns(columns, false);

            for (int i = 0; i < currentItems_.Count; ++i)
            {
                string item = currentItems_[i];
                if (item == "." || item == "..")
                    continue;
                if (ExcludeFilesystemItem(item))
                    continue;

                string displayName = item.Substring(item.LastIndexOf('\\') + 1);
                if (filter_.IsActive && !filter_.PassFilter(displayName))
                    continue;

                ImGuiCli.PushID(i);

                ImGuiCli.BeginGroup();

                Texture2D thumbnail = thumbCache_.GetOrCreateThumbnail(item);
                if (thumbnail != null)
                    ImGuiCli.ImageButton(thumbnail, new Vector2(itemSize - 20, itemSize - 16));
                else
                    ImGuiCli.Button("???", new Vector2(itemSize - 20, itemSize - 16));
                if (!ImGuiCli.IsPopupOpen() && ImGuiCli.BeginDragDropSource())
                {
                    if (Recents.Contains(CurrentDirectory))
                        Recents.Remove(CurrentDirectory);
                    Recents.Insert(0, CurrentDirectory);

                    ImGuiCli.SetDragDropPayload("U_ASSET", item);
                    if (thumbnail != null)
                        ImGuiCli.Image(thumbnail, new Vector2(128, 128));
                    ImGuiCli.Text(displayName);
                    ImGuiCli.EndDragDropSource();
                }

                if (ImGuiCli.IsItemDoubleClicked(0))
                {
                    System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(item);
                    if (dirInfo.Exists)
                        newDirectory_ = dirInfo.FullName;
                }
                else if (ImGuiCli.IsItemClicked(1))
                    ImGuiCli.OpenPopup("##asset_browser_popup");

                if (ImGuiCli.BeginPopup("##asset_browser_popup", ImGuiWindowFlags_.None))
                {
                    string ext = System.IO.Path.GetExtension(item);
                    if (ImGuiCli.MenuItem("Open (system)"))
                        System.Diagnostics.Process.Start(item);
                    //TODO: custom handlers
                    ImGuiCli.EndPopup();
                }

                if (asDetail_)
                    ImGuiCli.SameLine();
                ImGuiCli.TextWrapped(displayName);
                if (!asDetail_ && ImGuiCli.IsItemHovered())
                    ImGuiCli.SetTooltip(item);
                ImGuiCli.EndGroup();

                ImGuiCli.PopID();
                ImGuiCli.NextColumn();
            }

            ImGuiCli.EndChild();
        }

        void UpdateItems()
        {
            currentItems_ = System.IO.Directory.GetFileSystemEntries(CurrentDirectory).ToList();
            newDirectory_ = null;
        }

        void FillDirTree()
        {
            rootEntry_ = new DirEntry { directory_ = RootPath, shortName_ = new System.IO.DirectoryInfo(RootPath).Name };
            rootEntry_.PopulateChildren();
        }

        void DrawTree(DirEntry entry)
        {
            float imgSize = ImGuiCli.GetFontSize();

            for (int i = 0; i < entry.children_.Count; ++i)
            {
                string itemPath = entry.children_[i].directory_;
                string displayName = entry.children_[i].shortName_;
                string labelLessText = "##" + displayName;

                bool isLeaf = entry.children_[i].children_.Count == 0;
                bool isOpen = ImGuiCli.TreeNodeEx(labelLessText, isLeaf ? ImGuiTreeNodeFlags_.Leaf : ImGuiTreeNodeFlags_.None);

                ImGuiCli.SameLine();

                bool isSelected = CurrentDirectory == itemPath;
                bool wasSelected = isSelected;
                isSelected = ImGuiCli.Selectable(labelLessText + "_select", wasSelected, ImGuiSelectableFlags_.SpanAllColumns);

                Texture2D thumbnail = thumbCache_.GetOrCreateThumbnail(itemPath);
                if (thumbnail != null)
                {
                    ImGuiCli.SameLine();
                    ImGuiCli.Image(thumbnail, new Vector2(imgSize, imgSize));
                }
                ImGuiCli.SameLine();
                ImGuiCli.Text(displayName);
                if (isSelected != wasSelected)
                {
                    CurrentDirectory = itemPath;
                    UpdateItems();
                }

                if (isOpen)
                {
                    if (!isLeaf)
                        DrawTree(entry.children_[i]);
                    ImGuiCli.TreePop();
                }
            }
        }
    }
}
