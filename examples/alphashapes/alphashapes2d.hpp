AlphaSimplex2D::	    
AlphaSimplex2D(const ::Vertex& v): alpha_(0), attached_(false)
{
	for (int i = 0; i < 3; ++i)
		if (v.face()->vertex(i)->point() == v.point())
			Parent::add(v.face()->vertex(i));
}

AlphaSimplex2D::	    
AlphaSimplex2D(const Edge& e): attached_(false)
{
    Face_handle f = e.first;
	for (int i = 0; i < 3; ++i)
		if (i != e.second)
			Parent::add(f->vertex(i));
}

AlphaSimplex2D::	    
AlphaSimplex2D(const Edge& e, const SimplexSet& simplices, const Delaunay& Dt): attached_(false)
{
    Face_handle f = e.first;
	for (int i = 0; i < 3; ++i)
		if (i != e.second)
			Parent::add(f->vertex(i));

	Face_handle o = f->neighbor(e.second);
	int oi = o->index(f);

	VertexSet::const_iterator v = Parent::vertices().begin();
	const Point& p1 = (*v++)->point();
	const Point& p2 = (*v)->point();
	
	attached_ = false;
	if (!Dt.is_infinite(f->vertex(e.second)) &&
        CGAL::side_of_bounded_circle(p1, p2, 
									 f->vertex(e.second)->point()) == CGAL::ON_BOUNDED_SIDE)
		attached_ = true;
	else if (!Dt.is_infinite(o->vertex(oi)) &&
             CGAL::side_of_bounded_circle(p1, p2,
										  o->vertex(oi)->point()) == CGAL::ON_BOUNDED_SIDE)
		attached_ = true;
	else
		alpha_ = squared_radius(p1, p2);

	if (attached_)
	{
		if (Dt.is_infinite(f))
			alpha_ = simplices.find(AlphaSimplex2D(*o))->alpha();
		else if (Dt.is_infinite(o))
			alpha_ = simplices.find(AlphaSimplex2D(*f))->alpha();
		else
			alpha_ = std::min(simplices.find(AlphaSimplex2D(*f))->alpha(), 
                              simplices.find(AlphaSimplex2D(*o))->alpha());
	}
}

AlphaSimplex2D::	    
AlphaSimplex2D(const Face& f): attached_(false)
{
	for (int i = 0; i < 3; ++i)
		Parent::add(f.vertex(i));
	VertexSet::const_iterator v = Parent::vertices().begin();
	Point p1 = (*v++)->point();
	Point p2 = (*v++)->point();
	Point p3 = (*v)->point();
	alpha_ = CGAL::squared_radius(p1, p2, p3);
}

AlphaSimplex2D::Cycle
AlphaSimplex2D::boundary() const
{
	Cycle bdry;
	Parent::Cycle pbdry = Parent::boundary();
	for (Parent::Cycle::const_iterator cur = pbdry.begin(); cur != pbdry.end(); ++cur)
		bdry.push_back(*cur);
	return bdry;
}


bool 
AlphaSimplex2D::AlphaOrder::
operator()(const AlphaSimplex2D& first, const AlphaSimplex2D& second) const
{
	if (first.alpha() == second.alpha())
		return (first.dimension() < second.dimension());
	else
		return (first.alpha() < second.alpha()); 
}

std::ostream& 
AlphaSimplex2D::
operator<<(std::ostream& out) const
{
	for (VertexSet::const_iterator cur = Parent::vertices().begin(); 
								   cur != Parent::vertices().end(); ++cur)
		out << **cur << ", ";
	out << "value = " << value();

	return out;
}


void fill_alpha_order(const Delaunay& Dt, AlphaSimplex2DVector& alpha_order)
{
	// Compute all simplices with their alpha values and attachment information
	AlphaSimplex2D::SimplexSet simplices;
	for(Face_iterator cur = Dt.finite_faces_begin(); cur != Dt.finite_faces_end(); ++cur)
		simplices.insert(AlphaSimplex2D(*cur));
	rInfo("Faces inserted");
	for(Edge_iterator cur = Dt.finite_edges_begin(); cur != Dt.finite_edges_end(); ++cur)
		simplices.insert(AlphaSimplex2D(*cur, simplices, Dt));
	rInfo("Edges inserted");
	for(Vertex_iterator cur = Dt.finite_vertices_begin(); cur != Dt.finite_vertices_end(); ++cur)
		simplices.insert(AlphaSimplex2D(*cur));
	rInfo("Vertices inserted");
    
	// Sort simplices by their alpha values
	alpha_order.resize(simplices.size());
	std::copy(simplices.begin(), simplices.end(), alpha_order.begin());
	std::sort(alpha_order.begin(), alpha_order.end(), AlphaSimplex2D::AlphaOrder());
}

