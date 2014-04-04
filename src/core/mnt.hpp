#ifndef __MNT_HPP__
#define __MNT_HPP__

#include <osg/Geode>
////////////////////////////////////////////////////////////////////////////////
class MNT
{
public :
	typedef unsigned char byte;

	MNT();
	~MNT();

	bool charge( const char* nom_fichier, const char* type_fichier );
	unsigned int  get_numVertices()  { return dim_x*dim_y; }
	int  get_altitude(const int x, const int y)  { return altitudes[x+y*dim_x]; }
	osg::Node* MNT::getNode();

	void sauve_log( char* nom_fichier_log, char* nom_fichier_tga );
	bool sauve_partie( char* nom_fichier, int xpos, int ypos, int nb_pt_x, int nb_pt_y );
	bool sauve_partie_XML( char* nom_fichier, int xpos, int ypos, int nb_pt_x, int nb_pt_y);

	bool est_charge() { return mnt_charge; }
	int  get_dim_x()  { return dim_x; }
	int  get_dim_y()  { return dim_y; }
	float get_x_noeud_NO() { return x_noeud_NO; }
	float get_y_noeud_NO() { return y_noeud_NO; }

private:
	char	nom_chantier[500];
	char	unites_xy[20];
	float	precision_xy;
	int		dim_x, dim_y;
	float	x_noeud_NO, y_noeud_NO;
	float	pas_x, pas_y;
	char	unites_z[20];
	float	precision_z;

	int		NODATA_value;

	int		*altitudes;
	bool	mnt_charge;

public :
	byte	*image;
};
////////////////////////////////////////////////////////////////////////////////
#endif // __MNT_HPP__
